#include <flgl.h>

#include "util.h"
#include "Deck.h"
#include "PokerDriver.h"

#include <iostream>



typedef long double Money;

typedef enum {
    PSTATE_DEAL = 0,
    PSTATE_BET_CHECK,
    PSTATE_BET_OPEN,
    PSTATE_DISCARD,
    PSTATE_SHOW,
} pokerstate_e;

struct PokerPlayerController;

struct PokerPlayer {
    size_t index;
    PokerPlayerController* controller;
    Money stack;
    Money bet;
    Deck hand;
    bool in;
    Money charge(Money amt) {stack -= amt; assert(stack > 0. && "overcharged player"); return amt;}
    Money charge_to_bet(Money newbet) {
        Money diff = charge(newbet - this->bet);
        this->bet = newbet;
        return diff;
    }
    Money charge_all() {
        Money cash = this->charge(stack);
        bet += cash;
        return cash;
    }
    void award(Money payout) {stack += payout;}
    void end_round() {
        bet = 0.f; in = true; hand = Deck::new_empty();
    }
};

struct PlayerList : public std::vector<PokerPlayer> {
    PlayerList() {turn = 0;}
    ~PlayerList() {
        for (auto& p : *this) delete p.controller;
    }
    void add(PokerPlayerController* player, Money buyin = 10.) {
        this->push_back(PokerPlayer{this->size(), player, buyin, 0., Deck::new_empty(), true});
    }
    void set_turn(size_t t) {turn = t;}
    size_t get_turn() const {return turn;}
    size_t num_in(PokerPlayer** one = 0) {
        size_t r = 0; 
        for (auto& p : *this) {
            if (p.in) {
                r++;
                if (one) *one = &p;
            }
        } 
        return r;
    }
    bool any_in() {return num_in() > 0;}
    PokerPlayer* one_in() {PokerPlayer* res; size_t ni = num_in(&res); return ni == 1 ? res : 0;}

    PokerPlayer& get(size_t idx) {assert(idx < this->size() && "oob player get"); return this->at(idx);}
    PokerPlayer const& get(size_t idx) const {assert(idx < this->size() && "oob player get"); return this->at(idx);}
    PokerPlayer* next() {
        PokerPlayer* res = &this->at(turn);
        turn = (turn + 1) % this->size();
        return res;
    }
    PokerPlayer* next_under(const Money call) {
        PokerPlayer* const first = next();
        PokerPlayer* n = first;
        do {
            if (n->bet < call) return n;
            n = next();
        } while (n != first);
        return 0;
    }
private:
    size_t turn;
};

struct PokerState {
    PokerState(PlayerList& incoming) : state(PSTATE_DEAL), deck(Deck::new_shuffled()), bet(0.), pot(0.), players(incoming) {}
    pokerstate_e state;
    Deck deck;
    Money bet;
    Money pot;
    PlayerList& players;
};

struct PokerBetAction {
    PokerBetAction(size_t slf) : self(slf) {}
    virtual void perform(PokerState& game) = 0;
protected:
    size_t self;
};
struct CheckAction : public PokerBetAction {
    CheckAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        assert(game.bet == game.players.get(self).bet && "can't check if your bet doesn't match, call raise or fold");
    }
};
struct CallAction : public PokerBetAction {
    CallAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        assert(game.bet > 0. && "call with no bet open");
        assert(game.bet > game.players.get(self).bet && "call w no bet increase, check instead");
        Money cash = game.players.get(self).charge_to_bet(game.bet);
        game.pot += cash;
    }
};
struct RaiseAction : public PokerBetAction {
    Money bet;
    RaiseAction(size_t self, Money b) : PokerBetAction(self), bet(b) {}
    virtual void perform(PokerState& game) override final {
        assert(bet > game.bet && "raise invalid");
        Money cash = game.players.get(self).charge_to_bet(bet);
        game.pot += cash;
        game.bet = bet;
    }
};
struct FoldAction : public PokerBetAction {
    FoldAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        game.players.get(self).in = false;
    }
};
struct AllInAction : public PokerBetAction {
    AllInAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        Money cash = game.players.get(self).charge_all();
        game.pot += cash;
        game.bet = (game.players.get(self).bet > game.bet) ? game.players.get(self).bet : game.bet;
    }
};


struct PokerPlayerController {
    typedef struct Result {enum {CONTROL_OK = 0, CONTROL_BUSY} code;} DiscardResult, ShowResult;
    struct BetResult : public Result {
        BetResult(PokerBetAction* act) : action(act) {code = act == 0 ? CONTROL_BUSY : CONTROL_OK;}
        PokerBetAction* action;
    };
    virtual BetResult bet(PokerState const& game, PokerPlayer const& player) = 0;
    virtual DiscardResult discard(PokerState const& game, PokerPlayer const& player) = 0;
    virtual ShowResult show(PokerState const& game, PokerPlayer const& player) {
        assert(player.hand.size() == 5 && "if a players hand is bigger than a poker hand, you must override show() to select what cards to play");
        player.hand.mark_all();
        return Result{Result::CONTROL_OK};
    }
};


struct PokerGame {
    PokerGame(PlayerList& incoming) : state(incoming) {}
    PokerState state;

    struct Result {
        PokerPlayer* winner;
        Money payout;
    };

    void deal() {
        assert(state.deck.size() == 52 && "deck not full");
        for (PokerPlayer& p : state.players) {
            assert(p.hand.size() == 0 && "players need to be reset first");
            p.hand = state.deck.deal(5);
        }
    }
    
    PokerPlayer* bet() {
        PokerPlayer* const first = state.players.next();
        PokerPlayer* player = first;
        do {
            PokerBetAction* action = player->controller->bet(state, *player).action;
            action->perform(state);
            delete action;
            if (state.bet > 0.) break;
            if (PokerPlayer* winner = state.players.one_in())
                return winner;
            player = state.players.next();
        } while (player != first);

        while ((player = state.players.next_under(state.bet))) {
            PokerBetAction* action = player->controller->bet(state, *player).action;
            action->perform(state);
            delete action;
            if (PokerPlayer* winner = state.players.one_in())
                return winner;
        }
        return 0;
    }
    
    void discard() {
        for (PokerPlayer& p : state.players) {
            p.controller->discard(state, p);
            Deck disc = p.hand.get_marked();
            p.hand -= disc;
            for (auto c : disc) { (void)c;
                p.hand.add(state.deck.draw());
            }
        }
    }
    
    Result show() {
        hand_e best = HAND_HIGHCARD;
        PokerPlayer* bestp = 0;
        for (PokerPlayer& p : state.players) {
            if (!p.in) continue;
            p.controller->show(state, p);
            Deck d = p.hand.get_marked();
            assert(d.is_subset(p.hand) && "must show a subset of your hand, no cheating");
            hand_e hand = d.find_best_hand();
            if (hand > best) {
                best = hand; bestp = &p;
            }
            lg("player %lu had %s\n", p.index, hand_name(hand));
        }
        lg("\nPLAYER %lu WINS $%.2Lf!!!\n", bestp->index, state.pot);
        Result res{bestp, state.pot};
        return res;
    }
    

    Result run(size_t rounds = 1) {
        PokerPlayer* winner;
        deal();
        while (1) {
            winner = bet();
            if (winner) break;
            if (!(rounds--)) break;
            discard();
        }
        return show();
    }
};

struct ConsolePPC : public PokerPlayerController {
    ConsolePPC() : PokerPlayerController() {}
    virtual BetResult bet(PokerState const& game, PokerPlayer const& player) override final {
        std::cout << "Player " << player.index << ", time to bet. here is your hand:\n";
        player.hand.print();
        if (game.bet == player.bet) {
            std::cout << "you can 'check' or 'bet <n>' to open / raise. ";
        } else if (game.bet > player.bet) {
            std::cout << "you can 'call' (you owe " << game.bet - player.bet << "), 'fold' or raise with 'bet <n>'. ";
        }
        std::string inp; std::cin >> inp;
        if (inp == "check") {
            return new CheckAction(player.index);
        } else if (inp == "call") {
            return new CallAction(player.index);
        } else if (inp == "fold") {
            return new FoldAction(player.index);
        } else if (inp == "bet") {
            Money amt; std::cin >> amt;
            return new RaiseAction(player.index, player.bet + amt);
        }
        return new FoldAction(player.index);
    }
    virtual DiscardResult discard(PokerState const& game, PokerPlayer const& player) override final {
        std::cout << "Player " << player.index << ", time to discard. ";
        size_t i;
        Deck display = player.hand;
        do {
            std::cout << "here is your hand:\n";
            display.print();
            std::cout << "enter an idx to discard (0 thru n-1 top to bot) or 42 to stop: ";
            std::cin >> i;
            if (i == 42) break;
            Card pull = display.remove(i);
            player.hand.mark(player.hand.find(pull));
        } while (1);
        return DiscardResult{Result::CONTROL_OK};
    }
};




int main() {
    
    PlayerList players;

    players.add(new ConsolePPC());
    players.add(new ConsolePPC());
    players.add(new ConsolePPC());

    PokerGame game(players);

    game.run();

    return 0;
}































void test_interactive() {
    Deck deck;

    lg("before shuff:\n");
    deck.print();
    lg("\n\nafter shuff:\n");
    deck.shuffle();
    deck.print();
    while (1) {
        lg("draw? ");
        std::cin.get();
        auto hand = deck.deal(7);

        nl();
        lg("hand:\n");
        hand.print();
        lg("\n and the best hand there is %s\n", hand_name(hand.find_best_hand()));

    }

}

void test_cut() {
    Deck deck = Deck::new_deck();
    deck.print();
    nl();nl();
    deck.cut();
    deck.print();
}

void test_construct() {
    {Deck d = Deck::new_deck(); tassert(d.size() == 52);}
    {Deck d = Deck::new_shuffled(); tassert(d.size() == 52);}
}

void test_detection() {
    for (hand_e hand = HAND_HIGHCARD; hand < HAND_LAST; hand = hand_next(hand)) {
        Deck deck = Deck::new_hand(hand);
        lg("hand (%s) is: \n", hand_name(hand));
        deck.print(); 
        tassert(deck.find_best_hand() == hand);
        nl(); nl();
    }
}