#pragma once
#include "Deck.hpp"
#include "Card.hpp"
#include <vector>
#include <unordered_set>


struct PlayerState {
    int id;
    int total_score; // The Global game score

    // Round specific sate
    std::vector<Card> active_cards;
    int multiplier;
    int flat_bonus;
    bool has_second_chance;
    bool is_frozen;
    bool is_active_in_round;

    void reset_for_round();
    int calculate_round_score() const;
    bool has_card_value(int val) const;

};

class GameState{


public:

    GameState(int num_players);
    void setup_round(std::mt19937& rng);
    void apply_card(int player_idx, Card card, std::mt19937& rng);

    std::vector<PlayerState> players;
    Deck deck;
    int current_player_idx;
    bool round_active;

private:
    int get_random_active_opponent(int currect_idx, std::mt19937& rng) const;

};