#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <random>

static size_t rand_int(size_t a, size_t b) {
    static std::random_device rd;  // Non-deterministic random source
    static std::mt19937 gen(rd()); // Mersenne Twister PRNG seeded with rd()
    std::uniform_int_distribution<size_t> dist(a, b);
    return dist(gen);
}
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

typedef enum {
    SUITBMP_NONE = 0,
    SUITBMP_HEARTS = 1,
    SUITBMP_DIAMONDS = 2,
    SUITBMP_SPADES = 4,
    SUITBMP_CLUBS = 8,
    SUITBMP_LAST = 16,
} suit_bmp_e;

static suit_bmp_e suit_to_bmp(suit_e suit) {
    static const suit_bmp_e suits[] = {
        SUITBMP_HEARTS,
        SUITBMP_DIAMONDS,
        SUITBMP_SPADES,
        SUITBMP_CLUBS
    };
    return suits[suit];
}

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

typedef enum {
    HAND_HIGHCARD = 0,
    HAND_PAIR,
    HAND_2PAIR,
    HAND_3OFAKIND,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULLHOUSE,
    HAND_4OFAKIND,
    HAND_STRAIGHT_FLUSH,
    HAND_ROYAL_FLUSH,
} hand_e;

static const char* hand_name(hand_e hand) {
    static const char* names[] = {
        "HIGHCARD",
        "PAIR",
        "TWO PAIR",
        "THREEOFAKIND",
        "STRAIGHT",
        "FLUSH",
        "FULLHOUSE",
        "FOUROFAKIND",
        "STRAIGHT_FLUSH",
        "ROYAL_FLUSH",
    };
    return names[hand];
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

    static hand_e better_hand(hand_e a, hand_e b) {
        return a > b ? a : b;
    }

    hand_e find_best_hand() {
        hand_e power = HAND_HIGHCARD;

        

        uint32_t suit_ctr[SUIT_LAST] = {0};
        struct {
            uint32_t count;
            uint32_t suitmap;
        } rank_ctr[RANK_LAST] = {0};

        for (auto card : *this) {
            suit_ctr[card.suit]++;
            if (suit_ctr[card.suit] > 4) {
                power = better_hand(power, HAND_FLUSH);
            }
            rank_ctr[card.rank].count++;
            rank_ctr[card.rank].suitmap |= suit_to_bmp(card.suit);
        }

        bool pair = false;
        bool trip = false;
        size_t consec = 0;
        size_t cursuit = ~0;
        for (size_t r = 0; r < RANK_LAST; r++) {
            switch (rank_ctr[r].count) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                power = better_hand(power, pair ? HAND_2PAIR : HAND_PAIR);
                pair = true;
                break;
            case 3:
                power = better_hand(power, HAND_3OFAKIND);
                trip = true;
                break;
            case 4:
                power = better_hand(power, HAND_4OFAKIND);
                break;
            default:
                break;
            }

            if (rank_ctr[r].count) {
                /* have > 0 of rank r */
                if (consec == 0) {
                    /* first of a potential consec string */
                    cursuit = ~0;
                }
                consec++;
                cursuit &= rank_ctr[r].suitmap;
                if (consec > 4) {
                    power = better_hand(power, cursuit ? HAND_STRAIGHT_FLUSH : HAND_STRAIGHT);
                }
            } else {
                /* have none of rank r */
                cursuit = ~0;
                consec = 0;
            }
        }

        if (pair && trip) {
            power = better_hand(power, HAND_FULLHOUSE);
        }

        return power;
    }

    static Deck get_str8flush() {
        Deck hand(true);
        hand.push_back(Card{RANK_10, SUIT_HEARTS});
        hand.push_back(Card{RANK_JACK, SUIT_HEARTS});
        hand.push_back(Card{RANK_QUEEN, SUIT_HEARTS});
        hand.push_back(Card{RANK_KING, SUIT_HEARTS});
        hand.push_back(Card{RANK_ACE, SUIT_HEARTS});
        return hand;
    }
    static Deck get_flush() {
        Deck hand(true);
        hand.push_back(Card{RANK_10, SUIT_HEARTS});
        hand.push_back(Card{RANK_JACK, SUIT_HEARTS});
        hand.push_back(Card{RANK_QUEEN, SUIT_CLUBS});
        hand.push_back(Card{RANK_KING, SUIT_HEARTS});
        hand.push_back(Card{RANK_ACE, SUIT_HEARTS});
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
        lg("hand:\n");
        hand.print();
        lg("\n and the best hand there is %s\n", hand_name(hand.find_best_hand()));

    }

    // Deck sf = Deck::get_flush();
    // sf.print();

    // lg("best: %s\n", hand_name(sf.find_best_hand()));

    return 0;
}


