#include <flgl.h>

#include "util.h"
#include "Deck.h"
#include "PokerDriver.h"
#include "PokerGame.h"
#include "PokerAI.h"

#include <iostream>



int main() {

    PlayerList players;

    players.add(new ConsolePlayer());
    players.add(new ConsolePlayer());
    players.add(new ConsolePlayer());

    PokerGame game(players);

    game.run().print();

    // Driver* dr = new PokerDriver();

    // dr->start();

    // delete dr;


    return 0;
}









