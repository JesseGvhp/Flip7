#pragma once
#include "Card.hpp"
#include <vector>

class GameState; // Forward declaration to break circular include

class Policy {
public:
    virtual ~Policy() = default;

    // Decide whether to draw ("NOCH EINE" = true) or stay ("STOPP" = false)
    virtual bool should_draw(const GameState& state, int player_idx) const = 0;

    // Choose which player index from valid_targets receives card_type
    // (FREEZE, FLIP_THREE, or duplicate SECOND_CHANCE)
    virtual int choose_action_target(
        const GameState& state,
        int acting_player_idx,
        CardType card_type,
        const std::vector<int>& valid_targets) const;
};

class ThresholdPolicy : public Policy {
public:
    explicit ThresholdPolicy(int threshold);
    bool should_draw(const GameState& state, int player_idx) const override;

private:
    int stop_threshold;
};