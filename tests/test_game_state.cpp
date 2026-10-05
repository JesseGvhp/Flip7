#include <gtest/gtest.h>
#include <random>
#include "GameState.hpp"
#include "Card.hpp"


TEST(PlayerStateTest, ResetForRoundCleansState) {
    PlayerState player{0, 50, {{CardType::NUMBER, 5}}, 4, 10, true, true, false};

    player.reset_for_round();

    EXPECT_TRUE(player.active_cards.empty());
    EXPECT_EQ(player.multiplier, 1);
    EXPECT_EQ(player.flat_bonus, 0);
    EXPECT_FALSE(player.has_second_chance);
    EXPECT_FALSE(player.is_frozen);
    EXPECT_TRUE(player.is_active_in_round);
    EXPECT_EQ(player.total_score, 50); // Total global score must persist
}

TEST(PlayerStateTest, HasCardValueIdentifiesDuplicates) {
    PlayerState player;
    player.reset_for_round();

    player.active_cards.push_back({CardType::NUMBER, 7});
    player.active_cards.push_back({CardType::NUMBER, 2});

    EXPECT_TRUE(player.has_card_value(7));
    EXPECT_TRUE(player.has_card_value(2));
    EXPECT_FALSE(player.has_card_value(5));
}

TEST(PlayerStateTest, CalculateRoundScoreFormula) {
    PlayerState player;
    player.reset_for_round();

    player.active_cards.push_back({CardType::NUMBER, 4});
    player.active_cards.push_back({CardType::NUMBER, 6}); // Sum = 10
    player.multiplier = 2;
    player.flat_bonus = 5;

    // (Sum * Multiplier) + Flat Bonus = (10 * 2) + 5 = 25
    EXPECT_EQ(player.calculate_round_score(), 25);
}

TEST(PlayerStateTest, CalculateRoundScoreFlipSevenBonus) {
    PlayerState player;
    player.reset_for_round();

    // 7 distinct cards summing to 28
    for (int i = 1; i <= 7; ++i) {
        player.active_cards.push_back({CardType::NUMBER, i});
    }

    // Hand size == 7 grants +15 Flip 7 bonus: 28 + 15 = 43
    EXPECT_EQ(player.calculate_round_score(), 43);
}

TEST(PlayerStateTest, CalculateRoundScoreBustedVsFrozen) {
    PlayerState player;
    player.reset_for_round();
    player.active_cards.push_back({CardType::NUMBER, 10});

    // Case 1: Busted and not frozen -> 0 points
    player.is_active_in_round = false;
    player.is_frozen = false;
    EXPECT_EQ(player.calculate_round_score(), 0);

    // Case 2: Frozen player retains points even when inactive
    player.is_frozen = true;
    EXPECT_EQ(player.calculate_round_score(), 10);
}