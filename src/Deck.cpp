#include "Deck.h"

const char* rank_name(rank_e rank) {
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

suit_bmp_e suit_to_bmp(suit_e suit) {
    static const suit_bmp_e suits[] = {
        SUITBMP_HEARTS,
        SUITBMP_DIAMONDS,
        SUITBMP_SPADES,
        SUITBMP_CLUBS
    };
    return suits[suit];
}

const char* suit_name(suit_e suit) {
    static const char* suit_names[] = {
        "HEARTS",
        "DIAMONDS",
        "SPADES",
        "CLUBS",
    };
    return suit_names[suit];
}

void Card::print() {
    lg("%s of %s", rank_name(this->rank), suit_name(this->suit));
}

const char* hand_name(hand_e hand) {
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

Deck::Deck(bool empty) {
    if (empty) return;
    for (suit_e s = SUIT_HEARTS; s < SUIT_LAST; s = suit_next(s)) {
        for (rank_e r = RANK_2; r < RANK_LAST; r = rank_next(r)) {
            this->push_back(Card{r,s,false});
        }
    }
}

size_t Deck::size() const {return this->std::vector<Card>::size();}

std::vector<Card>::iterator Deck::begin() {
    return this->std::vector<Card>::begin();
}

std::vector<Card>::iterator Deck::end() {
    return this->std::vector<Card>::end();
}

std::vector<Card>::const_iterator Deck::begin() const {
    return this->std::vector<Card>::cbegin();
}

std::vector<Card>::const_iterator Deck::end() const {
    return this->std::vector<Card>::cend();
}

Deck Deck::new_empty() {return Deck(true);}

Deck Deck::new_deck() {return Deck();}

Deck Deck::new_shuffled(uint32_t N) {
    Deck deck;
    deck.shuffle(N);
    return deck;
}

void Deck::swap(size_t a, size_t b) {
    assert(a < this->size() && b < this->size());
    if (a == b) return;
    Card t = this->at(a);
    (*this)[a] = (*this)[b];
    (*this)[b] = t;
}

Card Deck::remove(size_t i) {
    assert(i < this->size());
    Card res = this->at(i);
    for (size_t j = i; j < this->size() - 1; j++) {
        this->swap(j, j+1);
    }
    this->pop_back();
    return res;
}

size_t Deck::find(Card card) const {
    for (size_t i = 0; i < this->size(); i++) {
        if (this->at(i) == card) return i;
    }
    return -1;
}

void Deck::cut() {
    if (this->size() < 2) return;
    const size_t split = this->size() / 2;
    for (size_t i = 0; i < split; i++) {
        this->swap(i, i + split);
    }
}

Card Deck::draw() {
    Card res = this->back();
    this->pop_back();
    return res;
}

Card Deck::peek() const {
    return this->back();
}

Card Deck::draw_random() {
    size_t idx = rand_int(0, this->size()-1);
    if (!(idx < this->size())) {
        return Card{RANK_LAST,SUIT_LAST,false};
    }
    Card res = this->at(idx);
    this->erase(std::next(this->begin(), idx));
    return res;
}

void Deck::add(Card card) {
    this->push_back(card);
}

void Deck::add(rank_e rank, suit_e suit) {
    this->add(Card{rank, suit, false});
}

void Deck::shuffle(uint32_t N) {
    for (uint32_t i = 0; i < N; i++) {
        this->push_back(this->draw_random());
    }
}

Deck Deck::deal(size_t const N) {
    Deck hand(true);
    for (size_t i = 0; i < N; i++) {
        if (this->empty()) break;
        hand.push_back(this->draw());
    }
    return hand;
}

bool Deck::contains(Card card) const {
    for (auto c : *this) {
        if (card == c) {
            return true;
        }
    }
    return false;
}

bool Deck::is_subset(Deck const &other) const
{
    for (auto c : *this) {
        if (!other.contains(c)) return false;
    }
    return true;
}

Deck &Deck::operator+=(Deck const &other) {
    for (auto c : other) {
        this->add(c);
    }
    return *this;
}

Deck &Deck::operator-=(Deck const &other) {
    for (auto c : other) {
        size_t idx = this->find(c);
        if (idx < this->size()) this->remove(idx);
    }
    return *this;
}

Deck Deck::get_marked() const {
    Deck res = Deck::new_empty();
    for (auto card : *this) {
        if (card.mark) {
            res.add(card);
        }
    }
    return res;
}

void Deck::mark_all(bool mk) const {
    for (auto& card : *this) {
        card.mark = mk;
    }
}

void Deck::mark(size_t i, bool mk) const {
    this->at(i).mark = mk;
}

hand_e Deck::find_best_hand() const {
    hand_e power = HAND_HIGHCARD;

    uint32_t suit_ctr[SUIT_LAST] = {0};
    struct {
        uint32_t count;
        uint32_t suitmap;
    } rank_ctr[RANK_LAST] = {0,0};

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
                power = better_hand(power, cursuit ? (r == RANK_ACE ? HAND_ROYAL_FLUSH : HAND_STRAIGHT_FLUSH) : HAND_STRAIGHT);
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

void Deck::print() const {
    for (auto card : *this) {
        card.print(); lg("\n");
    }
}



Deck Deck::new_hand(hand_e hand) {
    Deck res(true);

    switch(hand) {
    case HAND_HIGHCARD:
        res.push_back(Card{RANK_2, SUIT_CLUBS, false});
        res.push_back(Card{RANK_4, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_7, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_PAIR:
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_4, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_7, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_2PAIR:
        res.push_back(Card{RANK_2, SUIT_CLUBS, false});
        res.push_back(Card{RANK_2, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_3OFAKIND:
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_10, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_7, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_SPADES, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_STRAIGHT:
        res.push_back(Card{RANK_5, SUIT_CLUBS, false});
        res.push_back(Card{RANK_6, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_7, SUIT_HEARTS, false});
        res.push_back(Card{RANK_8, SUIT_SPADES, false});
        res.push_back(Card{RANK_9, SUIT_CLUBS, false});
        break;
    case HAND_FLUSH:
        res.push_back(Card{RANK_10, SUIT_HEARTS, false});
        res.push_back(Card{RANK_JACK, SUIT_HEARTS, false});
        res.push_back(Card{RANK_7, SUIT_HEARTS, false});
        res.push_back(Card{RANK_KING, SUIT_HEARTS, false});
        res.push_back(Card{RANK_ACE, SUIT_HEARTS, false});
        break;
    case HAND_FULLHOUSE:
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_10, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_SPADES, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_4OFAKIND:
        res.push_back(Card{RANK_10, SUIT_CLUBS, false});
        res.push_back(Card{RANK_10, SUIT_DIAMONDS, false});
        res.push_back(Card{RANK_10, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_SPADES, false});
        res.push_back(Card{RANK_QUEEN, SUIT_CLUBS, false});
        break;
    case HAND_STRAIGHT_FLUSH:
        res.push_back(Card{RANK_9, SUIT_HEARTS, false});
        res.push_back(Card{RANK_10, SUIT_HEARTS, false});
        res.push_back(Card{RANK_JACK, SUIT_HEARTS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_HEARTS, false});
        res.push_back(Card{RANK_KING, SUIT_HEARTS, false});
        break;
    case HAND_ROYAL_FLUSH:
        res.push_back(Card{RANK_10, SUIT_HEARTS, false});
        res.push_back(Card{RANK_JACK, SUIT_HEARTS, false});
        res.push_back(Card{RANK_QUEEN, SUIT_HEARTS, false});
        res.push_back(Card{RANK_KING, SUIT_HEARTS, false});
        res.push_back(Card{RANK_ACE, SUIT_HEARTS, false});
        break;
    default:
        break;
    }

    return res;
}

