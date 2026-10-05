#include "Deck.hpp"
#include <algorithm>
#include <stdexcept>

Deck::Deck() {
    initialize();
}

void Deck::initialize() {
    cards.clear();
    // 79 number cards: 12x12, 11x11, ..., 1x1, 1x0
    for (int i = 1; i <= 12; ++i) {
        for (int j = 0; j < i; ++j) {
            cards.push_back({CardType::NUMBER, i});
        }
    }
    cards.push_back({CardType::NUMBER, 0}); // 1x0

    // 9 Action cards
    for (int i = 0; i < 3; ++i) {
        cards.push_back({CardType::FREEZE, 0});
        cards.push_back({CardType::SECOND_CHANCE, 0});
        cards.push_back({CardType::FLIP_THREE, 0});
    }

    // 6 Bonus cards (+2, +4, +6, +8, +10, x2)
    // Check the bonus point distribution.

    cards.push_back({CardType::MULTIPLIER_X2, 0});

    for (int i = 1; i <= 5; ++i) {
        cards.push_back({CardType::BONUS_POINTS, 2*i}); 
    }
}

void Deck::shuffle(std::mt19937& rng) {
    std::shuffle(cards.begin(), cards.end(), rng);
}

Card Deck::draw() {
    if (cards.empty()) {
        throw std::runtime_error("Deck is empty!");
    }
    Card c = cards.back();
    cards.pop_back();
    return c;
}

bool Deck::is_empty() const { return cards.empty(); }
size_t Deck::size() const { return cards.size(); }