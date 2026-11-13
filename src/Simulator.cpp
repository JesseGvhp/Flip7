#include "Simulator.hpp"
#include <thread>
#include <numeric>
#include <stdexcept>
#include <algorithm>

Simulator::Simulator(std::vector<const Policy*> policies, int win_score)
    : player_policies(std::move(policies)), target_win_score(win_score) 
{
    if (player_policies.empty()) {
        throw std::invalid_argument("Simulator requires at least 1 player policy.");
    }
    for (const auto* policy : player_policies) {
        if (!policy) {
            throw std::invalid_argument("Policy pointer cannot be null.");
        }
    }
}

GameResult Simulator::run_single_game(std::mt19937& rng, int initial_starting_player) const {
    const int num_players = static_cast<int>(player_policies.size());
    GameState state(num_players, rng);

    if (initial_starting_player >= 0 && initial_starting_player < num_players) {
        state.starting_player_idx = initial_starting_player;
    } else {
        // Rule: Randomly determine starting player at the beginning of the game
        std::uniform_int_distribution<int> dist(0, num_players - 1);
        state.starting_player_idx = dist(rng);
    }

    int rounds = 0;
    int winner = -1;
    while ((winner = state.get_winner(target_win_score)) == -1) {
        state.play_round(rng, player_policies);
        rounds++;
    }

    GameResult result;
    result.winner_idx = winner;
    result.rounds_played = rounds;
    result.final_scores.reserve(num_players);
    for (const auto& p : state.players) {
        result.final_scores.push_back(p.total_score);
    }
    return result;
}

SimulationStats Simulator::run_parallel(
    int num_games,
    uint32_t base_seed,
    unsigned int num_threads) const 
{
    const int num_players = static_cast<int>(player_policies.size());
    SimulationStats total_stats;
    total_stats.num_games = num_games;
    total_stats.wins_per_player.assign(num_players, 0);
    total_stats.win_rates.assign(num_players, 0.0);
    total_stats.avg_final_scores.assign(num_players, 0.0);

    if (num_games <= 0) return total_stats;

    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
    }
    num_threads = std::min<unsigned int>(num_threads, static_cast<unsigned int>(num_games));

    struct WorkerAccum {
        std::vector<int> wins;
        std::vector<long long> score_sums;
        long long total_rounds = 0;
    };

    std::vector<WorkerAccum> worker_results(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    const int games_per_thread = num_games / static_cast<int>(num_threads);
    const int remainder = num_games % static_cast<int>(num_threads);

    int start_game_idx = 0;
    for (unsigned int t = 0; t < num_threads; ++t) {
        int count = games_per_thread + (static_cast<int>(t) < remainder ? 1 : 0);
        int end_game_idx = start_game_idx + count;

        worker_results[t].wins.assign(num_players, 0);
        worker_results[t].score_sums.assign(num_players, 0);

        threads.emplace_back([this, start_game_idx, end_game_idx, base_seed, num_players, &out = worker_results[t]]() {
            for (int g = start_game_idx; g < end_game_idx; ++g) {
                // Seed uniquely per game ID for deterministic parallel execution
                std::seed_seq seq{base_seed, static_cast<uint32_t>(g)};
                std::mt19937 rng(seq);

                GameResult res = run_single_game(rng);
                out.wins[res.winner_idx]++;
                out.total_rounds += res.rounds_played;
                for (int i = 0; i < num_players; ++i) {
                    out.score_sums[i] += res.final_scores[i];
                }
            }
        });

        start_game_idx = end_game_idx;
    }

    for (auto& th : threads) {
        th.join();
    }

    long long grand_total_rounds = 0;
    std::vector<long long> grand_score_sums(num_players, 0);

    for (unsigned int t = 0; t < num_threads; ++t) {
        grand_total_rounds += worker_results[t].total_rounds;
        for (int i = 0; i < num_players; ++i) {
            total_stats.wins_per_player[i] += worker_results[t].wins[i];
            grand_score_sums[i] += worker_results[t].score_sums[i];
        }
    }

    for (int i = 0; i < num_players; ++i) {
        total_stats.win_rates[i] = static_cast<double>(total_stats.wins_per_player[i]) / num_games;
        total_stats.avg_final_scores[i] = static_cast<double>(grand_score_sums[i]) / num_games;
    }
    total_stats.avg_rounds_per_game = static_cast<double>(grand_total_rounds) / num_games;

    return total_stats;
}