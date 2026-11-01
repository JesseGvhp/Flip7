#include <gtest/gtest.h>
#include <memory>
#include "Policy.hpp"
#include "GameState.hpp"
#include "Card.hpp"

// Test 1: Player with an empty hand (sum = 0) should draw when threshold > 0
TEST(ThresholdPolicyTest, DrawsWhenHandIsEmpty) {
    GameState state(1);
    state.players[0].reset_for_round();

    ThresholdPolicy policy(15);
    EXPECT_TRUE(policy.should_draw(state, 0));
}

// Test 2: Player draws when number sum is strictly below the threshold
TEST(ThresholdPolicyTest, DrawsWhenSumBelowThreshold) {
    GameState state(1);
    state.players[0].reset_for_round();
    state.players[0].active_cards.push_back({CardType::NUMBER, 6});
    state.players[0].active_cards.push_back({CardType::NUMBER, 8}); // Sum = 14

    ThresholdPolicy policy(15);
    EXPECT_TRUE(policy.should_draw(state, 0));
}

// Test 3: Boundary condition - Player stops when sum equals the threshold
TEST(ThresholdPolicyTest, StopsWhenSumEqualsThreshold) {
    GameState state(1);
    state.players[0].reset_for_round();
    state.players[0].active_cards.push_back({CardType::NUMBER, 7});
    state.players[0].active_cards.push_back({CardType::NUMBER, 8}); // Sum = 15

    ThresholdPolicy policy(15);
    EXPECT_FALSE(policy.should_draw(state, 0));
}

// Test 4: Player stops when number sum exceeds the threshold
TEST(ThresholdPolicyTest, StopsWhenSumExceedsThreshold) {
    GameState state(1);
    state.players[0].reset_for_round();
    state.players[0].active_cards.push_back({CardType::NUMBER, 10});
    state.players[0].active_cards.push_back({CardType::NUMBER, 12}); // Sum = 22

    ThresholdPolicy policy(15);
    EXPECT_FALSE(policy.should_draw(state, 0));
}

// Test 5: Threshold of 0 immediately stops even on a 0-value number card or empty hand
TEST(ThresholdPolicyTest, ZeroThresholdAlwaysStops) {
    GameState state(1);
    state.players[0].reset_for_round();

    ThresholdPolicy policy(0);
    EXPECT_FALSE(policy.should_draw(state, 0));

    state.players[0].active_cards.push_back({CardType::NUMBER, 0});
    EXPECT_FALSE(policy.should_draw(state, 0));
}

// Test 6: Evaluates the correct player_idx in a multi-player GameState
TEST(ThresholdPolicyTest, EvaluatesCorrectPlayerIndex) {
    GameState state(2);
    state.players[0].reset_for_round();
    state.players[1].reset_for_round();

    // Player 0 has sum = 20 (>= 15), Player 1 has sum = 5 (< 15)
    state.players[0].active_cards.push_back({CardType::NUMBER, 8});
    state.players[0].active_cards.push_back({CardType::NUMBER, 12});
    state.players[1].active_cards.push_back({CardType::NUMBER, 5});

    ThresholdPolicy policy(15);
    EXPECT_FALSE(policy.should_draw(state, 0));
    EXPECT_TRUE(policy.should_draw(state, 1));
}

// Test 7: Polymorphic call through base Policy pointer
TEST(ThresholdPolicyTest, WorksViaBaseClassPointer) {
    GameState state(1);
    state.players[0].reset_for_round();
    state.players[0].active_cards.push_back({CardType::NUMBER, 11});

    std::unique_ptr<Policy> policy = std::make_unique<ThresholdPolicy>(12);
    EXPECT_TRUE(policy->should_draw(state, 0));

    state.players[0].active_cards.push_back({CardType::NUMBER, 1}); // Sum = 12
    EXPECT_FALSE(policy->should_draw(state, 0));
}