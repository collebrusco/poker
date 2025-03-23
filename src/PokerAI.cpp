#include "PokerAI.h"



PokerPlayerController::BetResult BasicAIPlayer::bet(PokerState const &game, PokerPlayer const &player) {
    return BetResult(0);
}

PokerPlayerController::DiscardResult BasicAIPlayer::discard(PokerState const &game, PokerPlayer const &player) {
    return DiscardResult();
}


