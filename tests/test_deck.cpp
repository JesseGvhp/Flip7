#include <gtest/gtest.h>
#include <random> 
#include "Deck.hpp"
#include <map>





// Test 1: This verifies, that the Deck initializes with the correct size and composition
TEST(DeckTest, InializesWithCorrectSize){
    Deck deck;
    EXPECT_EQ(deck.size(), 94u);
    EXPECT_FALSE(deck.is_empty());
}

// Test 2: Verifies, drawing decrements size and eventually empties the deck
TEST(DeckTest, DrawReducesSizeUntilEmpty){
    Deck deck;

    constexpr size_t total_cards = 94;


    for (size_t i = 0; i < total_cards; ++i) {
        EXPECT_EQ(deck.size(), total_cards -i);

        Card c = deck.draw();
        
        //Values for calid cards are within expected range
        EXPECT_GE(c.value, 0);
        EXPECT_LE(c.value, 12);
    }

    EXPECT_EQ(deck.size(), 0u);
    EXPECT_TRUE(deck.is_empty());

}

// Test 3: Verify exception is thrown on an empty draw
TEST(DeckTest, DrawThowsWhenEmpty) {
    Deck deck;
    while (!deck.is_empty()){
        deck.draw();
    }
    
    //Expect std::runtime_error
    EXPECT_THROW(deck.draw(), std::runtime_error);
}


// Test 4: Verify shuffle retains total card count
TEST(DeckTest, ShufflePreservesDeckSize) {
    Deck deck;
    std::mt19937 rng(1337); // Fixed seed for reproducible tests

    deck.shuffle(rng);

    EXPECT_EQ(deck.size(), 94u);
    EXPECT_FALSE(deck.is_empty());
}

// Test 5: Verfiy re-initialization resets deck
TEST(DeckTest, ReinitResetsState) {
    Deck deck;
    deck.draw();
    deck.draw();
    EXPECT_EQ(deck.size(), 92u);
    
    //Reinit the deck
    deck.initialize();
    EXPECT_EQ(deck.size(), 94u);
}


TEST(DeckTest, TestCardDistribution){
    Deck deck;

    std::map<int, int> number_counts;
    std::map<int, int> bonus_counts;

    int freeze_count = 0;
    int second_chance_count = 0;
    int flip_three_count = 0;
    int multiplier_count = 0;

    // Draining the deck and keeping track of the counts
    while (!deck.is_empty())
    {
        Card c = deck.draw();

        switch (c.type) {
            case CardType::NUMBER:
                number_counts[c.value]++;
                break;
            case CardType::FREEZE:
                EXPECT_EQ(c.value, 0);
                freeze_count++;
                break;
            case CardType::SECOND_CHANCE:
                EXPECT_EQ(c.value, 0);
                second_chance_count++;
                break;
            case CardType::FLIP_THREE:
                EXPECT_EQ(c.value, 0);
                flip_three_count++;
                break;
            case CardType::MULTIPLIER_X2:
                EXPECT_EQ(c.value, 0);
                multiplier_count++;
                break;
            case CardType::BONUS_POINTS:
                bonus_counts[c.value]++;
                break;
        }
    }


    // Verify number card distribution: exactly 1x 0-value card
    EXPECT_EQ(number_counts[0], 1);

    // Verify triangular distribution: 1x 1, 2x 2, ..., 12x 12
    for (int i = 1; i <= 12; ++i) {
        EXPECT_EQ(number_counts[i], i);
    }

    // Verify Action cards (3 of each)
    EXPECT_EQ(freeze_count, 3);
    EXPECT_EQ(second_chance_count, 3);
    EXPECT_EQ(flip_three_count, 3);

    // Verify the bonus cars (+2, +4, +6, +8, +10, x2)
    EXPECT_EQ(multiplier_count, 1);

    for(int i = 1; i <= 5; ++i) {
        EXPECT_EQ(bonus_counts[2*i], 1);
    } 
}