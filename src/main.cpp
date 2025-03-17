#include <flgl.h>

#include "util.h"
#include "Deck.h"


#include <iostream>

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

int main() {
    
    // test_detection();

    // Deck h = Deck::new_hand(HAND_ROYAL_FLUSH);

    // lg ("hand should be royal flush, is %s\n", hand_name(h.find_best_hand()));

    return 0;
}


