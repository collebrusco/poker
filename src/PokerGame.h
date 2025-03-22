/** 
 * PokerGame.h 
 * poker
 * created 03/22/25 by frank collebrusco
 */
#ifndef POKER_GAME_H
#define POKER_GAME_H
#include <iostream>
#include "util.h"
#include "Deck.h"


typedef long double Money;

struct PokerPlayerController;

struct PokerPlayer {
    size_t index;
    PokerPlayerController* controller;
    Money stack;
    Money bet;
    Deck hand;
    bool in;
    Money charge(Money amt);
    Money charge_to_bet(Money newbet);
    Money charge_all();
    void award(Money payout);
    void end_round();
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

static inline pokerFSMinput_e operator&(pokerFSMinput_e const& a, pokerFSMinput_e const& b) {
    return ((pokerFSMinput_e)(((unsigned)a) & ((unsigned)b)));
}
static inline pokerFSMinput_e operator|(pokerFSMinput_e const& a, pokerFSMinput_e const& b) {
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
    const char* get_name() const;
    void next(pokerFSMinput_e input);
private:
    void next_DEAL(pokerFSMinput_e input);
    void next_PLAYER_RESET_INIT(pokerFSMinput_e input);
    void next_PLAYER_RESET_DISC(pokerFSMinput_e input);
    void next_PLAYER_RESET_SHOW(pokerFSMinput_e input);
    void next_BET_CHECK(pokerFSMinput_e input);
    void next_BET_OPEN(pokerFSMinput_e input);
    void next_ADV_CHECK(pokerFSMinput_e input);
    void next_ADV_OPEN(pokerFSMinput_e input);
    void next_ROUND_CHECK(pokerFSMinput_e input);
    void next_DISCARD(pokerFSMinput_e input);
    void next_DISCARD_ADV(pokerFSMinput_e input);
    void next_SHOW_ADV(pokerFSMinput_e input);
    void next_SHOW(pokerFSMinput_e input);
};

struct PlayerList : public std::vector<PokerPlayer> {
    PlayerList(size_t f = 0);
    ~PlayerList();
    void add(PokerPlayerController* player, Money buyin = 10.);
    void set_turn(size_t t);
    size_t get_turn() const;
    void set_first(size_t f);
    size_t get_first() const;
    size_t num_in(PokerPlayer** one = 0);
    bool any_in();
    PokerPlayer* one_in();
    void reset();
    void bring_all_in();
    PokerPlayer& get(size_t idx);
    PokerPlayer const& get(size_t idx) const;
    PokerPlayer& cur();
    PokerPlayer& first();
    PokerPlayer* next();
    PokerPlayer* next_under(const Money call);
private:
    size_t _first, turn;
};

struct PokerState : public PokerFiniteState {
    PokerState(PlayerList& incoming, size_t rounds = 2);
    Deck deck;
    Money bet;
    Money pot;
    size_t round;
    PlayerList& players;
};

struct PokerBetAction {
    inline PokerBetAction(size_t slf) : self(slf) {}
    virtual void perform(PokerState& game) = 0;
protected:
    size_t self;
};
struct CheckAction : public PokerBetAction {
    CheckAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final;
};
struct CallAction : public PokerBetAction {
    CallAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final;
};
struct RaiseAction : public PokerBetAction {
    Money bet;
    RaiseAction(size_t self, Money b) : PokerBetAction(self), bet(b) {}
    virtual void perform(PokerState& game) override final;
};
struct FoldAction : public PokerBetAction {
    FoldAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final;
};
struct AllInAction : public PokerBetAction {
    AllInAction(size_t self) : PokerBetAction(self) {}
    virtual void perform(PokerState& game) override final;
};


struct PokerPlayerController {
    typedef struct Result {enum {CONTROL_OK = 0, CONTROL_BUSY} code;} DiscardResult, ShowResult;
    struct BetResult : public Result {
        inline BetResult(PokerBetAction* act) : action(act) {code = act == 0 ? CONTROL_BUSY : CONTROL_OK;}
        PokerBetAction* action;
    };
    virtual BetResult bet(PokerState const& game, PokerPlayer const& player) = 0;
    virtual DiscardResult discard(PokerState const& game, PokerPlayer const& player) = 0;
    virtual ShowResult show(PokerState const& game, PokerPlayer const& player);
};

// typedef enum {
//     INIT,
//     STATE_CHANGE,

// } poker_event_e;

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
        void print() const;
    } result{Result::OK, 0, 0.};

    void print() const;

    pokerFSMinput_e execute();

    pokerFSMinput_e busy();
    pokerFSMinput_e ready(pokerFSMinput_e inp = INP_NONE);

    pokerFSMinput_e exec_DEAL();
    pokerFSMinput_e exec_PLAYER_RESET();
    pokerFSMinput_e exec_BET_CHECK();
    pokerFSMinput_e exec_BET_OPEN();
    pokerFSMinput_e exec_ADV_CHECK();
    pokerFSMinput_e exec_ADV_OPEN();
    pokerFSMinput_e exec_ROUND_CHECK();
    pokerFSMinput_e exec_DISCARD();
    pokerFSMinput_e exec_SHOW();
    pokerFSMinput_e exec_DISCARD_ADV();
    pokerFSMinput_e exec_SHOW_ADV();
    pokerFSMinput_e exec_END();

    Result step();
    Result step_until_busy();
    Result run();

};

struct ConsolePPC : public PokerPlayerController {
    ConsolePPC() : PokerPlayerController() {}
    virtual BetResult bet(PokerState const& game, PokerPlayer const& player) override final;
    virtual DiscardResult discard(PokerState const& game, PokerPlayer const& player) override final;
};



#endif /* POKER_GAME_H */
