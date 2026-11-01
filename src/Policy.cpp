#include "Policy.hpp"

ThresholdPolicy::ThresholdPolicy(int threshold) : stop_threshold(threshold) {}

bool ThresholdPolicy::should_draw(const GameState& state, int player_idx) {
    const auto& p = state.players[player_idx];
    int sum = 0;
    for (const auto& c : p.active_cards) {
        if (c.type == CardType::NUMBER) sum += c.value;
    }
    return sum < stop_threshold;
}