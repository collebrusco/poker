/** 
 * PokerDriver.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef POKER_DRIVER_H
#define POKER_DRIVER_H
#include "Driver.h"
#include "Deck.h"

class PokerDriver : public GameDriver {
    virtual void user_create() override final;
    virtual void user_update(float dt, Keyboard const& kb, Mouse const& mouse) override final;
    virtual void user_render() override final;
    virtual void user_destroy() override final;





};



void PokerDriver::user_create() {

}

void PokerDriver::user_update(float dt, Keyboard const& kb, Mouse const& mouse) {

}

void PokerDriver::user_render() {

}

void PokerDriver::user_destroy() {

}




#endif /* POKER_DRIVER_H */
