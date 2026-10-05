#include <gtest/gtest.h>
#include <random>
#include "GameState.hpp"
#include "Card.hpp"



// This test is for validating that we have the right state at the beginning of each round 
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


// This test sample probes if has_card_value() works correctly
TEST(PlayerStateTest, HasCardValueIdentifiesDuplicates) {
    PlayerState player;
    player.reset_for_round();

    player.active_cards.push_back({CardType::NUMBER, 7});
    player.active_cards.push_back({CardType::NUMBER, 2});

    EXPECT_TRUE(player.has_card_value(7));
    EXPECT_TRUE(player.has_card_value(2));
    EXPECT_FALSE(player.has_card_value(5));
}


// This test probes, if the round score formula is correct. 
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


// This test probes, if the Flip7 bonus at the end is correctly implemented.
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


// This test pobes, if the case distinction between busted and frozen works.
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




TEST(GameStateTest, ApplyCardNumberAddsOrBusts) {
    GameState game(2);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();

    // 1. Draw distinct numbers
    game.apply_card(0, {CardType::NUMBER, 5}, rng);
    EXPECT_EQ(game.players[0].active_cards.size(), 1u);
    EXPECT_TRUE(game.players[0].is_active_in_round);

    // 2. Draw duplicate number without Second Chance -> Bust
    game.apply_card(0, {CardType::NUMBER, 5}, rng);
    EXPECT_FALSE(game.players[0].is_active_in_round);
}

TEST(GameStateTest, ApplyCardSecondChanceAbsorbsDuplicate) {
    GameState game(2);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();

    game.apply_card(0, {CardType::NUMBER, 8}, rng);
    game.apply_card(0, {CardType::SECOND_CHANCE, 0}, rng);
    EXPECT_TRUE(game.players[0].has_second_chance);

    // Duplicate card should consume Second Chance and prevent busting
    game.apply_card(0, {CardType::NUMBER, 8}, rng);
    EXPECT_TRUE(game.players[0].is_active_in_round);
    EXPECT_FALSE(game.players[0].has_second_chance);
}

TEST(GameStateTest, ApplyCardSecondChancePassesToOpponentIfAlreadyHeld) {
    GameState game(2);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();
    game.players[1].reset_for_round();

    // Player 0 receives first Second Chance
    game.apply_card(0, {CardType::SECOND_CHANCE, 0}, rng);
    EXPECT_TRUE(game.players[0].has_second_chance);
    EXPECT_FALSE(game.players[1].has_second_chance);

    // Player 0 receives duplicate Second Chance -> must pass to Player 1
    game.apply_card(0, {CardType::SECOND_CHANCE, 0}, rng);
    EXPECT_TRUE(game.players[0].has_second_chance);
    EXPECT_TRUE(game.players[1].has_second_chance);
}

TEST(GameStateTest, ApplyCardMultiplierAndBonus) {
    GameState game(1);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();

    game.apply_card(0, {CardType::MULTIPLIER_X2, 0}, rng);
    EXPECT_EQ(game.players[0].multiplier, 2);

    game.apply_card(0, {CardType::BONUS_POINTS, 10}, rng);
    EXPECT_EQ(game.players[0].flat_bonus, 10);
}

TEST(GameStateTest, ApplyCardFreezeStopsOpponentActions) {
    GameState game(2);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();
    game.players[1].reset_for_round();

    // Player 0 plays FREEZE -> targets Player 1
    game.apply_card(0, {CardType::FREEZE, 0}, rng);
    EXPECT_TRUE(game.players[1].is_frozen);

    // Frozen player cannot receive further cards
    game.apply_card(1, {CardType::NUMBER, 9}, rng);
    EXPECT_TRUE(game.players[1].active_cards.empty());
}

TEST(GameStateTest, ApplyCardFlipSevenTerminatesRound) {
    GameState game(1);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();
    game.round_active = true;

    for (int i = 1; i <= 6; ++i) {
        game.apply_card(0, {CardType::NUMBER, i}, rng);
        EXPECT_TRUE(game.round_active);
    }

    // 7th distinct card triggers Flip 7 and ends the round immediately
    game.apply_card(0, {CardType::NUMBER, 7}, rng);
    EXPECT_FALSE(game.round_active);
}

TEST(GameStateTest, ApplyCardFlipThreeDealsThreeCards) {
    GameState game(2);
    std::mt19937 rng(42);
    game.players[0].reset_for_round();
    game.players[1].reset_for_round();
    game.deck.initialize();

    const size_t deck_size_before = game.deck.size();

    // Player 0 triggers FLIP_THREE on the only active opponent (Player 1)
    game.apply_card(0, {CardType::FLIP_THREE, 0}, rng);

    // Expect 3 cards drawn from the deck
    EXPECT_EQ(game.deck.size(), deck_size_before - 3u);
}