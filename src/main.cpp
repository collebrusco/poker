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
};

struct PlayerList : public std::vector<PokerPlayer> {
    void add(PokerPlayerController* player) {
        this->push_back(PokerPlayer{this->size(), player, 0., 0., Deck::new_empty(), true});
    }
    void set_turn(size_t t) {turn = t;}
    size_t get_turn() {return turn;}
    PokerPlayer& next() {
        turn = (turn + 1) % this->size();
        return this->at(turn);
    }
    PokerPlayer* next_under(const Money call) {
        PokerPlayer* const first = &next();
        PokerPlayer* n = first;
        do {
            if (n->bet < call) return n;
            n = &next();
        } while (n != first);
        return 0;
    }
private:
    size_t turn;
};

struct PokerState {
    PokerState(PlayerList& incoming) : state(PSTATE_DEAL), deck(Deck::new_shuffled()), pot(0.), players(incoming) {}
    pokerstate_e state;
    Deck deck;
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
    CheckAction(PokerPlayer* slf);
    virtual void perform(PokerState& game) override final;
};
struct CallAction : public PokerBetAction {
    CallAction(PokerPlayer* slf);
    virtual void perform(PokerState& game) override final;
};
struct RaiseAction : public PokerBetAction {
    RaiseAction(PokerPlayer* slf, Money bet);
    virtual void perform(PokerState& game) override final;
};
struct FoldAction : public PokerBetAction {
    FoldAction(PokerPlayer* slf);
    virtual void perform(PokerState& game) override final;
};
struct AllInAction : public PokerBetAction {
    AllInAction(PokerPlayer* slf);
    virtual void perform(PokerState& game) override final;
};


struct PokerPlayerController {
    PokerPlayerController(PokerPlayer* player) : player(*player) {}
    virtual PokerBetAction* bet(PokerState const& game) = 0;
    virtual Deck discard(PokerState const& game) = 0;
    virtual Deck show(PokerState const& game) = 0;
protected:
    PokerPlayer& player;
};

struct PokerGame {
    PokerGame(PlayerList& incoming) : state(incoming) {}
    PokerState state;

    void deal() {
        for (PokerPlayer& p : state.players) {
            p.hand = state.deck.deal(5);
        }
    }
    
    void bet() {
        PokerPlayer* const first = &state.players.next();
        PokerPlayer* player = first;
        do {
            PokerBetAction* action = player->controller->bet(state);
            action->perform(state);
            if (state.pot > 0.) break;
            player = &state.players.next();
        } while (player != first);

        while ((player = state.players.next_under(state.pot))) {
            PokerBetAction* action = player->controller->bet(state);
            action->perform(state);
        }

    }
    
    void discard() {
        for (PokerPlayer& p : state.players) {
            Deck disc = p.controller->discard(state);
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








int main() {
    

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