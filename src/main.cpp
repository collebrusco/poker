#include <flgl.h>

#include "util.h"
#include "Deck.h"
#include "PokerDriver.h"

#include <iostream>



typedef long double Money;

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
    PlayerList(size_t f = 0) {_first = turn = f; bring_all_in();}
    ~PlayerList() {
        for (auto& p : *this) delete p.controller;
    }
    void add(PokerPlayerController* player, Money buyin = 10.) {
        this->push_back(PokerPlayer{this->size(), player, buyin, 0., Deck::new_empty(), true});
    }
    void set_turn(size_t t) {turn = t;}
    size_t get_turn() const {return turn;}
    void set_first(size_t f) {_first = f;}
    size_t get_first() const {return _first;}
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
    void reset() {turn = _first;}
    void bring_all_in() {for (auto& p : *this) p.in = true;}
    PokerPlayer& get(size_t idx) {assert(idx < this->size() && "oob player get"); return this->at(idx);}
    PokerPlayer const& get(size_t idx) const {assert(idx < this->size() && "oob player get"); return this->at(idx);}
    PokerPlayer& cur() {return this->at(turn);}
    PokerPlayer& first() {return this->at(_first);}
    PokerPlayer* next() {
        do {
            turn = (turn + 1) % this->size();
        } while (!cur().in);
        return &cur();
    }
    PokerPlayer* next_under(const Money call) {
        PokerPlayer* const _first = &cur();
        PokerPlayer* n;
        do {
            n = next();
            if (n->bet < call) return n;
        } while (n != _first);
        return 0;
    }
private:
    size_t _first, turn;
};

typedef enum {
    INP_NONE            = 0x00,
    INP_ONE_LEFT        = 0x01,
    INP_CHECK           = 0x02,
    INP_BET             = 0x04,
    INP_PLAYER_FIRST    = 0x08,
    INP_PLAYER_NULL     = 0x10,
    INP_MORE_ROUNDS     = 0x20,
    INP_CONTROL_READY   = 0x40,
} pokerFSMinput_e;

inline pokerFSMinput_e operator&(pokerFSMinput_e const& a, pokerFSMinput_e const& b) {
    return ((pokerFSMinput_e)(((unsigned)a) & ((unsigned)b)));
}
inline pokerFSMinput_e operator|(pokerFSMinput_e const& a, pokerFSMinput_e const& b) {
    return ((pokerFSMinput_e)(((unsigned)a) | ((unsigned)b)));
}
static inline bool test_FSMinput(pokerFSMinput_e to_test, pokerFSMinput_e test) {
    return to_test & test;
}

struct PokerFiniteState {
    enum {
        DEAL = 0,
        PLAYER_RESET_INIT,
        PLAYER_RESET_DISC,
        PLAYER_RESET_SHOW,
        BET_CHECK,
        BET_OPEN,
        ADV_CHECK,
        ADV_OPEN,
        ROUND_CHECK,
        DISCARD,
        DISCARD_ADV,
        SHOW,
        SHOW_ADV,
        END,
    } state;
    const char* get_name() const {
        static const char* names[] = {
            "DEAL",
            "PLAYER_RESET_INIT",
            "PLAYER_RESET_DISC",
            "PLAYER_RESET_SHOW",
            "BET_CHECK",
            "BET_OPEN",
            "ADV_CHECK",
            "ADV_OPEN",
            "ROUND_CHECK",
            "DISCARD",
            "DISCARD_ADV",
            "SHOW",
            "SHOW_ADV",
            "END",
        }; return names[state];
    }
    void next(pokerFSMinput_e input) {
        switch (state) {
        case BET_CHECK:
        case BET_OPEN:
        case DISCARD:
        case SHOW:
            if (!test_FSMinput(input, INP_CONTROL_READY))
                return;
        default:
            break;
        }
        switch (state) {
        case DEAL:
            next_DEAL(input);
            break;
        case PLAYER_RESET_INIT:
            next_PLAYER_RESET_INIT(input);
            break;
        case PLAYER_RESET_DISC:
            next_PLAYER_RESET_DISC(input);
            break;
        case PLAYER_RESET_SHOW:
            next_PLAYER_RESET_SHOW(input);
            break;
        case BET_CHECK:
            next_BET_CHECK(input);
            break;
        case BET_OPEN:
            next_BET_OPEN(input);
            break;
        case ADV_CHECK:
            next_ADV_CHECK(input);
            break;
        case ADV_OPEN:
            next_ADV_OPEN(input);
            break;
        case ROUND_CHECK:
            next_ROUND_CHECK(input);
            break;
        case DISCARD:
            next_DISCARD(input);
            break;
        case SHOW:
            next_SHOW(input);
            break;
        case DISCARD_ADV:
            next_DISCARD_ADV(input);
            break;
        case SHOW_ADV:
            next_SHOW_ADV(input);
            break;
        default:
            break;
        }
    }
private:
    void next_DEAL(pokerFSMinput_e input) {
        state = PLAYER_RESET_INIT;
    }
    void next_PLAYER_RESET_INIT(pokerFSMinput_e input) {
        state = BET_CHECK;
    }
    void next_PLAYER_RESET_DISC(pokerFSMinput_e input) {
        state = DISCARD;
    }
    void next_PLAYER_RESET_SHOW(pokerFSMinput_e input) {
        state = END;
    }
    void next_BET_CHECK(pokerFSMinput_e input) {
        bool check = test_FSMinput(input, INP_CHECK);
        bool bet = test_FSMinput(input, INP_BET);
        assert(check ^ bet && "one of check or bet must have happened, not both or none");
        if (check)
            state = ADV_CHECK;
        if (bet)
            state = ADV_OPEN;
    }
    void next_BET_OPEN(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_ONE_LEFT))
            state = END;
        else
            state = ADV_OPEN;
    }
    void next_ADV_CHECK(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_PLAYER_FIRST))
            state = ROUND_CHECK;
        else
            state = BET_CHECK;
    }
    void next_ADV_OPEN(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_PLAYER_NULL))
            state = ROUND_CHECK;
        else
            state = BET_OPEN;
    }
    void next_ROUND_CHECK(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_MORE_ROUNDS))
            state = PLAYER_RESET_DISC;
        else
            state = PLAYER_RESET_SHOW;
    }
    void next_DISCARD(pokerFSMinput_e input) {
        state = DISCARD_ADV;
    }
    void next_DISCARD_ADV(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_PLAYER_FIRST))
            state = BET_CHECK;
        else
            state = DISCARD;

    }
    void next_SHOW_ADV(pokerFSMinput_e input) {
        if (test_FSMinput(input, INP_PLAYER_FIRST))
            state = END;
        else
            state = SHOW;
    }
    void next_SHOW(pokerFSMinput_e input) {
        state = SHOW_ADV;
    }
};

struct PokerState : public PokerFiniteState {
    PokerState(PlayerList& incoming, size_t rounds = 1) 
        : PokerFiniteState({PokerFiniteState::DEAL}), deck(Deck::new_shuffled()), bet(0.), pot(0.), round(rounds), players(incoming) {}
    Deck deck;
    Money bet;
    Money pot;
    size_t round;
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


struct PokerGame : public PokerState {
    PokerGame(PlayerList& incoming, size_t rounds = 2) : PokerState(incoming, rounds) {}

    struct Result {
        enum {
            BUSY = 0,
            OK,
            END,
        } status;
        PokerPlayer* winner;
        Money payout;
        void print() const {
            if (status == END) {
                printf("GAME OVER: PLAYER %lu WINS %.2Lf WITH A %s!\n", winner->index, payout, hand_name(winner->hand.find_best_hand()));
            } else {
                printf("game in progress, status %s\n", status == OK ? "OK" : "busy (waiting on a player)");
            }
        }
    } result{Result::OK, 0, 0.};

    void print() const {
        printf("\n\n====STATE INFO====\n");
        printf("state: %s; round: %lu; pot: $%.2Lf\n", this->get_name(), this->round, this->pot);
        printf("player %lu's turn (%lu first)\n", players.get_turn(), players.get_first());
        printf("they have %.2Lf bet now, %.2Lf in their stack, their hand:\n", players.cur().bet, players.cur().stack);
        players.cur().hand.print();
        printf("\n\n");
    }

    pokerFSMinput_e execute() {
        result.status = Result::OK;
        switch (state) {
        case DEAL:
            return exec_DEAL();
        case PLAYER_RESET_INIT:
        case PLAYER_RESET_DISC:
        case PLAYER_RESET_SHOW:
            return exec_PLAYER_RESET();
        case BET_CHECK:
            return exec_BET_CHECK();
        case BET_OPEN:
            return exec_BET_OPEN();
        case ADV_CHECK:
            return exec_ADV_CHECK();
        case ADV_OPEN:
            return exec_ADV_OPEN();
        case ROUND_CHECK:
            return exec_ROUND_CHECK();
        case DISCARD:
            return exec_DISCARD();
        case DISCARD_ADV:
            return exec_DISCARD_ADV();
        case SHOW:
            return exec_SHOW();            
        case SHOW_ADV:
            return exec_SHOW_ADV();
        case END:
            return exec_END();
        default: return INP_NONE;
        }
    }

    pokerFSMinput_e busy() {
        result.status = Result::BUSY;
        return INP_NONE;
    }

    pokerFSMinput_e ready(pokerFSMinput_e inp = INP_NONE) {
        return inp | INP_CONTROL_READY;
    }

    pokerFSMinput_e exec_DEAL() {
        assert(deck.size() == 52 && "deck not full");
        for (PokerPlayer& p : players) {
            assert(p.hand.size() == 0 && "players need to be reset first");
            p.hand = deck.deal(5);
        }
        return INP_NONE;
    }
    pokerFSMinput_e exec_PLAYER_RESET() {
        players.reset(); return INP_NONE;
    }
    pokerFSMinput_e exec_BET_CHECK() {
        PokerPlayerController::BetResult b = players.cur().controller->bet(*this, players.cur());
        if (b.code == b.CONTROL_BUSY) return busy();
        Money bet_before = this->bet;
        b.action->perform(*this);
        if (this->bet > bet_before)
            return ready(INP_BET);
        else
            return ready(INP_CHECK);
    }
    pokerFSMinput_e exec_BET_OPEN() {
        PokerPlayerController::BetResult b = players.cur().controller->bet(*this, players.cur());
        if (b.code == b.CONTROL_BUSY) return busy();
        b.action->perform(*this);
        if (players.one_in())
            return ready(INP_ONE_LEFT);
        else
            return ready(INP_NONE);
    }
    pokerFSMinput_e exec_ADV_CHECK() {
        players.next();
        return (players.get_turn() == players.get_first()) ? INP_PLAYER_FIRST : INP_NONE;
    }
    pokerFSMinput_e exec_ADV_OPEN() {
        return players.next_under(bet) ? INP_NONE : INP_PLAYER_NULL;
    }
    pokerFSMinput_e exec_ROUND_CHECK() {
        return --round ? INP_MORE_ROUNDS : INP_NONE;
    }
    pokerFSMinput_e exec_DISCARD() {
        PokerPlayerController::DiscardResult d = players.cur().controller->discard(*this, players.cur());
        if (d.code == d.CONTROL_BUSY) return busy();
        Deck disc = players.cur().hand.get_marked();
        players.cur().hand -= disc;
        for (auto c : disc) { (void)c;
            players.cur().hand.add(deck.draw());
        }
        return ready(INP_NONE);
    }
    pokerFSMinput_e exec_SHOW() {
        PokerPlayerController::ShowResult s;
        players.reset();
        players.cur().controller->discard(*this, players.cur());
        if (s.code == s.CONTROL_BUSY) return busy();
        return ready(INP_NONE);
    }
    pokerFSMinput_e exec_DISCARD_ADV() {
        return exec_ADV_CHECK();
    }
    pokerFSMinput_e exec_SHOW_ADV() {
        return exec_ADV_CHECK();
    }

    pokerFSMinput_e exec_END() {
        size_t besti = 0; hand_e best = HAND_HIGHCARD;
        for (auto& p : players) {
            if (p.in) {
                hand_e cur = p.hand.get_marked().find_best_hand();
                if (cur > best) {
                    best = cur;
                    besti = p.index;
                }
            }
        }
        result.winner = &players.get(besti);
        result.payout = pot;
        result.status = Result::END;
        return INP_NONE;
    }

    Result step() {
        print();
        next(execute());
        return result;
    }

    Result step_until_busy() {
        do {
            this->step();
        } while (result.status == Result::OK);
        return result;
    }

    Result run() {
        do {
            this->step();
        } while (result.status != Result::END);
        return result;
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

    game.run().print();


    return 0;
}









