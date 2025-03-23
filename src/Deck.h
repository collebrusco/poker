/** 
 * Deck.h 
 * poker
 * created 03/17/25 by frank collebrusco
 */
#ifndef DECK_H
#define DECK_H
#include "util.h"

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

static inline rank_e rank_next(rank_e rank) {
    if (rank == RANK_LAST) return rank;
    return (rank_e)(((unsigned)rank)+1);
}

const char* rank_name(rank_e rank);

typedef enum {
    SUIT_HEARTS = 0,
    SUIT_DIAMONDS,
    SUIT_SPADES,
    SUIT_CLUBS,
    SUIT_LAST,
} suit_e;

static inline suit_e suit_next(suit_e suit) {
    if (suit == SUIT_LAST) return suit;
    return (suit_e)(((unsigned)suit)+1);
}

typedef enum {
    SUITBMP_NONE = 0,
    SUITBMP_HEARTS = 1,
    SUITBMP_DIAMONDS = 2,
    SUITBMP_SPADES = 4,
    SUITBMP_CLUBS = 8,
    SUITBMP_LAST = 16,
} suit_bmp_e;

static inline suit_bmp_e suitbmp_next(suit_bmp_e suit) {
    if (suit == SUITBMP_LAST) return suit;
    return (suit_bmp_e)(((unsigned)suit)<<1);
}

suit_bmp_e suit_to_bmp(suit_e suit);

const char* suit_name(suit_e suit);

struct Card {
    rank_e rank;
    suit_e suit;
    mutable bool mark;
    void print();
};

bool inline operator==(Card const& a, Card const& b) {
    return a.rank == b.rank && a.suit == b.suit;
}

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
    HAND_LAST,
} hand_e;

static inline hand_e hand_next(hand_e hand) {
    if (hand == HAND_LAST) return hand;
    return (hand_e)(((unsigned)hand)+1);
}

const char* hand_name(hand_e hand);

static inline hand_e better_hand(hand_e a, hand_e b) {
    return a > b ? a : b;
}

struct Deck : private std::vector<Card> {
    Deck(bool empty = false);

    size_t size() const;
    std::vector<Card>::iterator begin();
    std::vector<Card>::iterator end();
    std::vector<Card>::const_iterator begin() const;
    std::vector<Card>::const_iterator end() const;

    static Deck new_empty();
    static Deck new_deck();
    static Deck new_shuffled(uint32_t N = 2048);
    static Deck new_hand(hand_e hand);

    void swap(size_t a, size_t b);
    Card remove(size_t i);
    size_t find(Card card) const;
    void cut();
    void shuffle(uint32_t N = 2048, bool cut = true);
    Card peek() const;
    Card draw();
    Card draw_random();
    void add(Card card);
    void add(rank_e rank, suit_e suit);
    Deck deal(size_t const N = 5);

    bool contains(Card card) const;
    bool is_subset(Deck const& other) const;
    Deck& operator+=(Deck const& other);
    Deck& operator-=(Deck const& other);
    Deck operator+(Deck const& other);
    Deck operator-(Deck const& other);
    Deck get_marked() const;
    Card get_highcard() const;
    void mark_all(bool mark = true) const;
    void mark(size_t i, bool mark = true) const;

    hand_e find_best_hand() const;
    
    void print() const;
    
};

struct DeckSet {
    inline DeckSet(Deck const& in, size_t n = 1) : deck(in), options(Deck::new_deck() - in), N(n) {assert(N==1 && "n > 1 unimplemented :(");}
    Deck const& deck;
    Deck const options;
    size_t const N;
    struct DeckSetIterator {
        std::vector<Card>::const_iterator it;
        DeckSet* home;
        inline DeckSetIterator(DeckSet* h, std::vector<Card>::const_iterator const& in) : home(h), it(in) {}
        inline DeckSetIterator& operator++() {++it; return *this;}
        inline Deck operator*() const {
            Deck res = home->deck; res.add(*it); return res;
        }
        inline bool operator!=(DeckSetIterator const& other) {
            return this->it != other.it;
        }
    };
    inline DeckSetIterator begin() {
        return DeckSetIterator(this, options.begin());
    }
    inline DeckSetIterator end() {
        return DeckSetIterator(this, options.end());
    }
};

#endif /* DECK_H */
