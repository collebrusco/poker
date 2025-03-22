#include <flgl.h>

#include "util.h"
#include "Deck.h"
#include "PokerDriver.h"
#include "PokerGame.h"

#include <iostream>



int main() {
    
    PlayerList players;

    players.add(new ConsolePPC());
    players.add(new ConsolePPC());
    players.add(new ConsolePPC());

    PokerGame game(players);

    game.run().print();


    return 0;
}









