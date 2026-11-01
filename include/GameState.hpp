#pragma once
#include "Deck.hpp"
#include "Card.hpp"
#include <vector>
#include <random>

struct PlayerState {
    int id = 0;
    int total_score = 0; // The global game score

    // Round-specific state
    std::vector<Card> active_cards{};
    int multiplier = 1;
    int flat_bonus = 0;
    bool has_second_chance = false;
    bool is_frozen = false;
    bool has_busted = false;
    bool is_active_in_round = false;

    void reset_for_round();
    int calculate_round_score() const;
    bool has_card_value(int val) const;
};

class GameState {
public:
    GameState(int num_players, std::mt19937& rng);
    GameState(int num_players); // Convenience constructor for deterministic unit tests

    void setup_round(std::mt19937& rng);
    void apply_card(int player_idx, Card card, std::mt19937& rng);
    Card draw_or_reshuffle(std::mt19937& rng);

    std::vector<PlayerState> players;
    Deck deck;
    int current_player_idx = 0;
    bool round_active = false;

private:
    int get_random_active_opponent(int current_idx, std::mt19937& rng, bool only_without_second_chance = false) const;
};