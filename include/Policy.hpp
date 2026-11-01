# pragma once
# include "GameState.hpp"


class Policy {
    // The class for the game policy
    public:
        virtual ~Policy() = default;
        virtual bool should_draw(const GameState& state, int player_idx) = 0;
};


//Stops if the sum of the cards is >= threshold

class ThresholdPolicy : public Policy {
    public:
        ThresholdPolicy(int threshold);
        bool should_draw(const GameState& state, int player_idx) override;

    private:
        int stop_threshold;
};