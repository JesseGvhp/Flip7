#pragma once
#include "Policy.hpp"
#include "GameState.hpp"
#include <vector>
#include <cstdint>

struct GameResult {
    int winner_idx = -1;
    int rounds_played = 0;
    std::vector<int> final_scores;
};

struct SimulationStats {
    int num_games = 0;
    std::vector<int> wins_per_player;
    std::vector<double> win_rates;
    std::vector<double> avg_final_scores;
    double avg_rounds_per_game = 0.0;
};

class Simulator {
public:
    Simulator(std::vector<const Policy*> policies, int win_score = 200);

    // Run a single game session on a given RNG
    GameResult run_single_game(std::mt19937& rng, int initial_starting_player = -1) const;

    // Run num_games in parallel across num_threads (0 = hardware_concurrency)
    SimulationStats run_parallel(
        int num_games,
        uint32_t base_seed = 42,
        unsigned int num_threads = 0) const;

private:
    std::vector<const Policy*> player_policies;
    int target_win_score;
};