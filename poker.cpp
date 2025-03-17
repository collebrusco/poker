#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define lg printf
static inline void nl() {lg("\n");}

typedef enum {
    RANK_2 = 0,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_10,
    RANK_JACK,
    RANK_QUEEN,
    RANK_KING,
    RANK_ACE,
    RANK_LAST,
} rank_e;

static const char* rank_name(rank_e rank) {
    static const char* rank_names[] = {
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "10",
        "JACK",
        "QUEEN",
        "KING",
        "ACE",
    };
    return rank_names[rank];
}

typedef enum {
    SUIT_HEARTS = 0,
    SUIT_DIAMONDS,
    SUIT_SPADES,
    SUIT_CLUBS,
    SUIT_LAST,
} suit_e;

static const char* suit_name(suit_e suit) {
    static const char* suit_names[] = {
        "HEARTS",
        "DIAMONDS",
        "SPADES",
        "CLUBS",
    };
    return suit_names[suit];
}

struct Card {
    rank_e rank;
    suit_e suit;
    void print() {
        lg("%s of %s", rank_name(this->rank), suit_name(this->suit));
    }
};

#include <random>

static size_t rand_int(size_t a, size_t b) {
    static std::random_device rd;  // Non-deterministic random source
    static std::mt19937 gen(rd()); // Mersenne Twister PRNG seeded with rd()
    std::uniform_int_distribution<size_t> dist(a, b);
    return dist(gen);
}

struct Deck : private std::vector<Card> {
    Deck(bool empty = false) {
        if (empty) return;
        for (rank_e r = RANK_2; r < RANK_LAST; r = (rank_e)(((int)r)+1)) {
            for (suit_e s = SUIT_HEARTS; s < SUIT_LAST; s = (suit_e)(((int)s)+1)) {
                this->push_back(Card{r,s});
            }
        }
    }

    Card draw() {
        size_t idx = rand_int(0, this->size()-1);
        if (!(idx < this->size())) {
            return Card{RANK_LAST,SUIT_LAST};
        }
        Card res = this->at(idx);
        this->erase(std::next(this->begin(), idx));
        return res;
    }

    Deck deal(size_t const N = 5) {
        Deck hand(true);
        for (size_t i = 0; i < N; i++) {
            if (this->empty()) break;
            hand.push_back(this->draw());
        }
        return hand;
    }

    void print() {
        for (auto card : *this) {
            card.print(); lg("\n");
        }
    }

};



#include <iostream>

int main() {
    
    Deck deck;

    deck.print();
    while (1) {
        lg("draw? ");
        std::cin.get();
        auto hand = deck.deal(7);

        nl();

        hand.print();

    }

    return 0;
}


