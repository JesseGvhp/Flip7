#include "Policy.hpp"
#include "GameState.hpp"
#include <algorithm>

int Policy::choose_action_target(
    const GameState& state,
    int acting_player_idx,
    CardType card_type,
    const std::vector<int>& valid_targets) const 
{
    if (valid_targets.empty()) return -1;

    // Default strategic heuristic:
    // - For harmful cards (FREEZE, FLIP_THREE): target the active opponent with the highest current total + round score.
    // - For helpful cards (duplicate SECOND_CHANCE): give to the active opponent with the lowest total + round score.
    int best_target = valid_targets.front();
    int best_metric = 0;
    bool first = true;

    for (int target_idx : valid_targets) {
        if (target_idx == acting_player_idx && valid_targets.size() > 1) {
            continue; // Prefer targeting opponents over self when possible
        }

        const auto& p = state.players[target_idx];
        int current_projected_score = p.total_score + p.calculate_round_score();

        if (first) {
            best_target = target_idx;
            best_metric = current_projected_score;
            first = false;
        } else if (card_type == CardType::SECOND_CHANCE) {
            if (current_projected_score < best_metric) {
                best_metric = current_projected_score;
                best_target = target_idx;
            }
        } else {
            // FREEZE or FLIP_THREE: target leader
            if (current_projected_score > best_metric) {
                best_metric = current_projected_score;
                best_target = target_idx;
            }
        }
    }
    return best_target;
}

ThresholdPolicy::ThresholdPolicy(int threshold) : stop_threshold(threshold) {}

bool ThresholdPolicy::should_draw(const GameState& state, int player_idx) const {
    const auto& p = state.players[player_idx];
    int sum = 0;
    for (const auto& c : p.active_cards) {
        if (c.type == CardType::NUMBER) sum += c.value;
    }
    return sum < stop_threshold;
}