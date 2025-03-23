/** 
 * PokerAI.h 
 * poker
 * created 03/22/25 by frank collebrusco
 */
#ifndef POKER_AI_H
#define POKER_AI_H
#include "PokerGame.h"

struct BasicAIPlayer : public PokerPlayerController {
    BasicAIPlayer() : PokerPlayerController() {}
    virtual BetResult bet(PokerState const& game, PokerPlayer const& player) override final;
    virtual DiscardResult discard(PokerState const& game, PokerPlayer const& player) override final;
};

#endif /* POKER_AI_H */
