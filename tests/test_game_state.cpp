#include <gtest/gtest.h>
#include <random>
#include "GameState.hpp"
#include "Card.hpp"

// Test 1: Validate clean state at the beginning of each round
TEST(PlayerStateTest, ResetForRoundCleansState) {
    PlayerState player{0, 50, {{CardType::NUMBER, 5}}, 4, 10, true, true, true, false};

    player.reset_for_round();

    EXPECT_TRUE(player.active_cards.empty());
    EXPECT_EQ(player.multiplier, 1);
    EXPECT_EQ(player.flat_bonus, 0);
    EXPECT_FALSE(player.has_second_chance);
    EXPECT_FALSE(player.is_frozen);
    EXPECT_FALSE(player.has_busted);
    EXPECT_TRUE(player.is_active_in_round);
    EXPECT_EQ(player.total_score, 50); // Total global score must persist
}

// Test 2: Verify has_card_value() identifies duplicates
TEST(PlayerStateTest, HasCardValueIdentifiesDuplicates) {
    PlayerState player;
    player.reset_for_round();

    player.active_cards.push_back({CardType::NUMBER, 7});
    player.active_cards.push_back({CardType::NUMBER, 2});

    EXPECT_TRUE(player.has_card_value(7));
    EXPECT_TRUE(player.has_card_value(2));
    EXPECT_FALSE(player.has_card_value(5));
}

// Test 3: Verify round score formula
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

// Test 4: Verify Flip 7 (+15) bonus
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

// Test 5: Verify scoring distinction between Voluntary Stay, Frozen, and Busted
TEST(PlayerStateTest, CalculateRoundScoreStayedVsFrozenVsBusted) {
    PlayerState player;
    player.reset_for_round();
    player.active_cards.push_back({CardType::NUMBER, 10});

    // Case 1: Voluntarily stayed (inactive, not frozen, not busted) -> retains 10 points
    player.is_active_in_round = false;
    player.is_frozen = false;
    player.has_busted = false;
    EXPECT_EQ(player.calculate_round_score(), 10);

    // Case 2: Frozen player (inactive, frozen, not busted) -> retains 10 points
    player.is_frozen = true;
    player.has_busted = false;
    EXPECT_EQ(player.calculate_round_score(), 10);

    // Case 3: Busted player -> 0 points
    player.is_frozen = false;
    player.has_busted = true;
    EXPECT_EQ(player.calculate_round_score(), 0);
}

TEST(GameStateTest, ApplyCardNumberAddsOrBusts) {
    GameState game(2);
    std::mt19937 rng(42);
    game.round_active = true;
    game.players[0].reset_for_round();

    // 1. Draw distinct number
    game.apply_card(0, {CardType::NUMBER, 5}, rng);
    EXPECT_EQ(game.players[0].active_cards.size(), 1u);
    EXPECT_TRUE(game.players[0].is_active_in_round);
    EXPECT_FALSE(game.players[0].has_busted);

    // 2. Draw duplicate number without Second Chance -> Bust
    game.apply_card(0, {CardType::NUMBER, 5}, rng);
    EXPECT_FALSE(game.players[0].is_active_in_round);
    EXPECT_TRUE(game.players[0].has_busted);
    EXPECT_EQ(game.players[0].calculate_round_score(), 0);
}

TEST(GameStateTest, ApplyCardSecondChanceAbsorbsDuplicate) {
    GameState game(2);
    std::mt19937 rng(42);
    game.round_active = true;
    game.players[0].reset_for_round();

    game.apply_card(0, {CardType::NUMBER, 8}, rng);
    game.apply_card(0, {CardType::SECOND_CHANCE, 0}, rng);
    EXPECT_TRUE(game.players[0].has_second_chance);

    // Duplicate card should consume Second Chance and prevent busting
    game.apply_card(0, {CardType::NUMBER, 8}, rng);
    EXPECT_TRUE(game.players[0].is_active_in_round);
    EXPECT_FALSE(game.players[0].has_busted);
    EXPECT_FALSE(game.players[0].has_second_chance);
}

TEST(GameStateTest, ApplyCardSecondChancePassesToEligibleOpponentWithoutOne) {
    GameState game(3);
    std::mt19937 rng(42);
    game.round_active = true;
    for (auto& p : game.players) p.reset_for_round();

    // Both Player 0 and Player 1 already hold Second Chance; Player 2 does not
    game.players[0].has_second_chance = true;
    game.players[1].has_second_chance = true;
    game.players[2].has_second_chance = false;

    // Player 0 draws a duplicate Second Chance -> must go to Player 2 (only eligible recipient)
    game.apply_card(0, {CardType::SECOND_CHANCE, 0}, rng);
    EXPECT_TRUE(game.players[2].has_second_chance);
}

TEST(GameStateTest, ApplyCardMultiplierAndBonus) {
    GameState game(1);
    std::mt19937 rng(42);
    game.round_active = true;
    game.players[0].reset_for_round();

    game.apply_card(0, {CardType::MULTIPLIER_X2, 0}, rng);
    EXPECT_EQ(game.players[0].multiplier, 2);

    game.apply_card(0, {CardType::BONUS_POINTS, 10}, rng);
    EXPECT_EQ(game.players[0].flat_bonus, 10);
}

TEST(GameStateTest, ApplyCardFreezeEliminatesTargetAndExcludesFromFutureTargeting) {
    GameState game(3);
    std::mt19937 rng(42);
    game.round_active = true;
    for (auto& p : game.players) p.reset_for_round();

    // Freeze Player 1 first by making Player 2 temporarily inactive
    game.players[2].is_active_in_round = false;
    game.apply_card(0, {CardType::FREEZE, 0}, rng);
    EXPECT_TRUE(game.players[1].is_frozen);
    EXPECT_FALSE(game.players[1].is_active_in_round);

    // Re-activate Player 2; another FREEZE from Player 0 must now target Player 2 (since Player 1 is already frozen)
    game.players[2].is_active_in_round = true;
    game.apply_card(0, {CardType::FREEZE, 0}, rng);
    EXPECT_TRUE(game.players[2].is_frozen);
    EXPECT_FALSE(game.players[2].is_active_in_round);
}

TEST(GameStateTest, ApplyCardFreezeTargetsSelfWhenOnlyActivePlayer) {
    GameState game(2);
    std::mt19937 rng(42);
    game.round_active = true;
    game.players[0].reset_for_round();
    game.players[1].reset_for_round();
    game.players[1].is_active_in_round = false; // Player 1 already stopped

    // Player 0 is the only active player left and draws FREEZE -> must freeze themselves
    game.apply_card(0, {CardType::FREEZE, 0}, rng);
    EXPECT_TRUE(game.players[0].is_frozen);
    EXPECT_FALSE(game.players[0].is_active_in_round);
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

TEST(GameStateTest, ApplyCardFlipThreeStopsDrawingEarlyOnBust) {
    GameState game(2);
    std::mt19937 rng(42);
    game.round_active = true;
    game.players[0].reset_for_round();
    game.players[1].reset_for_round();

    // Unshuffled deck layout from back to front:
    // - 6 Bonus cards (size 94 -> 88)
    // - 9 Action cards (size 88 -> 79)
    // - 1 Zero card (size 79 -> 78)
    // - 12 Twelve cards (size 78 -> 66)
    game.deck.initialize();
    while (game.deck.size() > 78) {
        game.deck.draw(); // Drain 6 bonus + 9 action + 1 zero card; top of deck is now twelve 12s!
    }

    // Give Player 1 a 12 in their active cards so the very first draw from FLIP_THREE causes a bust
    game.players[1].active_cards.push_back({CardType::NUMBER, 12});
    const size_t deck_size_before = game.deck.size();

    game.apply_card(0, {CardType::FLIP_THREE, 0}, rng);

    // Player 1 busted on the 1st card, so only 1 card should have been drawn (not 3!)
    EXPECT_TRUE(game.players[1].has_busted);
    EXPECT_EQ(game.deck.size(), deck_size_before - 1u);
}

TEST(GameStateTest, SetupRoundPersistsDeckAcrossRounds) {
    std::mt19937 rng(42);
    GameState game(2, rng);

    const size_t initial_size = game.deck.size();
    game.setup_round(rng);
    const size_t size_after_round_1 = game.deck.size();
    EXPECT_LT(size_after_round_1, initial_size);

    // Second round should continue drawing from the remaining deck without resetting back to 94
    game.setup_round(rng);
    EXPECT_LT(game.deck.size(), size_after_round_1);
}