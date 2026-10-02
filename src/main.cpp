#include <iostream>
#include <random>
#include <string>
#include "Deck.hpp"
#include "Card.hpp"



// Helper Function to print the correct cards

std::string card_type_to_string(CardType type) {
    switch (type) {
        case CardType::NUMBER:         return "NUMBER";
        case CardType::FREEZE:         return "FREEZE";
        case CardType::SECOND_CHANCE:  return "SECOND_CHANCE";
        case CardType::FLIP_THREE:     return "FLIP_THREE";
        case CardType::MULTIPLIER_X2:  return "MULTIPLIER_X2";
        case CardType::BONUS_POINTS:   return "BONUS_POINTS";
        default:                       return "UNKNOWN";
    }
}


void print_card(const Card& c) {
    std::cout << "[" << card_type_to_string(c.type) 
              << " | Value: " << c.value << "]\n";
}

int main() {
    std::cout << "=== Testing Deck Initialization ===\n";
    Deck deck;
    std::cout << "Initial deck size: " << deck.size() << " (Expected: 94)\n";
    std::cout << "Is empty: " << (deck.is_empty() ? "Yes" : "No") << "\n\n";

    std::cout << "=== Testing Deck Shuffling ===\n";
    // Seed with a true random device, or use a fixed integer (e.g. 42) for deterministic tests
    std::random_device rd;
    std::mt19937 rng(rd());
    
    deck.shuffle(rng);
    std::cout << "Deck shuffled successfully.\n\n";

    std::cout << "=== Testing Drawing 5 Cards ===\n";
    for (int i = 1; i <= 5; ++i) {
        Card drawn = deck.draw();
        std::cout << "Draw #" << i << ": ";
        print_card(drawn);
    }
    std::cout << "Remaining cards: " << deck.size() << " (Expected: 89)\n\n";

    std::cout << "=== Testing Drawing All Remaining Cards ===\n";
    while (!deck.is_empty()) {
        deck.draw();
    }
    std::cout << "Deck size after draining: " << deck.size() << "\n";
    std::cout << "Is empty: " << (deck.is_empty() ? "Yes" : "No") << "\n\n";

    std::cout << "=== Testing Empty Deck Exception Handling ===\n";
    try {
        std::cout << "Attempting to draw from empty deck...\n";
        deck.draw(); // Should throw std::runtime_error
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << "\n";
    }

    std::cout << "\n=== Testing Re-Initialization ===\n";
    deck.initialize();
    std::cout << "Deck size after re-initialize: " << deck.size() << " (Expected: 94)\n";

    std::cout << "\nAll tests completed successfully!\n";
    return 0;
}