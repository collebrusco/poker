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
    size_t get_turn() {return turn;}
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
    PokerBetAction(PokerPlayer* slf) : self(*slf) {}
    virtual void perform(PokerState& game) = 0;
protected:
    PokerPlayer& self;
};
struct CheckAction : public PokerBetAction {
    CheckAction(PokerPlayer* self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        assert(game.bet == self.bet && "can't check if your bet doesn't match, call raise or fold");
    }
};
struct CallAction : public PokerBetAction {
    CallAction(PokerPlayer* self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        assert(game.bet > 0. && "call with no bet open");
        assert(game.bet > self.bet && "call w no bet increase, check instead");
        Money cash = self.charge_to_bet(game.bet);
        game.pot += cash;
    }
};
struct RaiseAction : public PokerBetAction {
    Money bet;
    RaiseAction(PokerPlayer* self, Money b) : PokerBetAction(self), bet(b) {}
    virtual void perform(PokerState& game) override final {
        assert(bet > game.bet && "raise invalid");
        Money cash = self.charge_to_bet(bet);
        game.pot += cash;
        game.bet = bet;
    }
};
struct FoldAction : public PokerBetAction {
    FoldAction(PokerPlayer* self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        self.in = false;
    }
};
struct AllInAction : public PokerBetAction {
    AllInAction(PokerPlayer* self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final {
        Money cash = self.charge_all();
        game.pot += cash;
        game.bet = (self.bet > game.bet) ? self.bet : game.bet;
    }
};


struct PokerPlayerController {
    virtual PokerBetAction* bet(PokerState const& game, PokerPlayer& player) = 0;
    virtual Deck discard(PokerState const& game, PokerPlayer& player) = 0;
    virtual Deck show(PokerState const& game, PokerPlayer& player) = 0;
};

struct PokerGame {
    PokerGame(PlayerList& incoming) : state(incoming) {}
    PokerState state;

    void deal() {
        for (PokerPlayer& p : state.players) {
            assert(p.hand.size() == 0 && "players need to be reset first");
            p.hand = state.deck.deal(5);
        }
    }
    
    void bet() {
        PokerPlayer* const first = state.players.next();
        PokerPlayer* player = first;
        do {
            PokerBetAction* action = player->controller->bet(state, *player);
            action->perform(state);
            delete action;
            if (state.bet > 0.) break;
            player = state.players.next();
        } while (player != first);

        while ((player = state.players.next_under(state.bet))) {
            PokerBetAction* action = player->controller->bet(state, *player);
            action->perform(state);
            delete action;
        }

    }
    
    void discard() {
        for (PokerPlayer& p : state.players) {
            Deck disc = p.controller->discard(state, p);
            for (auto c : disc) { (void)c;
                p.hand.add(state.deck.draw());
            }
        }
    }
    
    void show() {
        hand_e best = HAND_HIGHCARD;
        size_t besti = -1;
        for (PokerPlayer& p : state.players) {
            hand_e hand = p.hand.find_best_hand();
            if (hand > best) {
                best = hand; besti = p.index;
            }
            lg("player %lu had %s\n", p.index, hand_name(hand));
        }
        lg("\nPLAYER %lu WINS $%.2Lf!!!\n", besti, state.pot);
    }
    

    void run(size_t rounds = 1) {
        deal();
        while (1) {
            bet();
            if (!(rounds--)) break;
            discard();
        }
        show();
    }
};



struct ConsolePPC : public PokerPlayerController {
    ConsolePPC() : PokerPlayerController() {}
    virtual PokerBetAction* bet(PokerState const& game, PokerPlayer& player) override final {
        std::cout << "Player " << player.index << ", time to bet. here is your hand:\n";
        player.hand.print();
        if (game.bet == player.bet) {
            std::cout << "you can 'check' or 'bet <n>' to open / raise. ";
        } else if (game.bet > player.bet) {
            std::cout << "you can 'call' (you owe " << game.bet - player.bet << "), 'fold' or raise with 'bet <n>'. ";
        }
        std::string inp; std::cin >> inp;
        if (inp == "check") {
            return new CheckAction(&player);
        } else if (inp == "call") {
            return new CallAction(&player);
        } else if (inp == "fold") {
            return new FoldAction(&player);
        } else if (inp == "bet") {
            Money amt; std::cin >> amt;
            return new RaiseAction(&player, player.bet + amt);
        }
        return new FoldAction(&player);
    }
    virtual Deck discard(PokerState const& game, PokerPlayer& player) override final {
        std::cout << "Player " << player.index << ", time to discard. ";
        size_t i;
        Deck res(true);
        do {
            std::cout << "here is your hand:\n";
            player.hand.print();
            std::cout << "enter an idx to discard (0 thru n-1 top to bot) or 42 to stop: ";
            std::cin >> i;
            if (i == 42) break;
            Card pull = player.hand.remove(i);
            res.add(pull);
        } while (1);
        return res;
    }
    virtual Deck show(PokerState const& game, PokerPlayer& player) override final {
        return player.hand;
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