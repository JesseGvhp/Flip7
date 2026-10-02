#pragma once
#include "Card.hpp"
#include <vector> 
#include <random>


class Deck {
    public:
        Deck();
        void initialize();
        void shuffle(std::mt19937& rng);
        Card draw();
        bool is_empty() const;
        size_t size() const;

    private:
        std::vector<Card> cards;
};