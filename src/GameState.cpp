#include "GameState.hpp"
#include <numeric>


void PlayerState::reset_for_round() {
    active_cards.clear();
    multiplier = 1;
    flat_bonus = 0;
    has_second_chance = false;
    is_frozen = false;
    is_active_in_round = true;
}

int PlayerState::calculate_round_score() const {

    // Busting logic
    if (!is_active_in_round && !is_frozen) return 0;


    int sum = 0;

    for (const auto& c : active_cards) {
        if (c.type == CardType::NUMBER) sum += c.value;
    }

    // Scoring order according to the rules: sum * multiplier + bonus
    int score = (sum * multiplier) + flat_bonus;


    // Flip7 check
    if (active_cards.size() == 7) {
        score += 15;
    }
    return score;
}

bool PlayerState::has_card_value(int val) const {
    for(const auto& c : active_cards) {
        if (c.type == CardType::NUMBER && c.value == val) return true;
    }
    return false;
}



//Constructor for GameState
// Check if the last argument (i.e. is_active_in_round should be set to false in the constructor)
GameState::GameState(int num_players) {
    for(int i = 0; i < num_players; ++i){
        players.push_back({i, 0, {}, 1, 0, false, false, false});
    }
}

void GameState::setup_round(std::mt19937& rng) {
    deck.initialize();
    deck.shuffle(rng);

    round_active = true;

    for (auto& p : players) p.reset_for_round();

    // Deal 1 card to everyone, according to the rules.
    for (size_t i =0; i < players.size(); ++i) {
        apply_card(i, deck.draw(), rng);
    }


    current_player_idx = 0;
}


int GameState::get_random_active_opponent(int current_idx, std::mt19937& rng) const {
    std::vector<int> targets;

    for (size_t i = 0; i < players.size(); ++i) {
        if (i != current_idx && players[i].is_active_in_round) {
            targets.push_back(i);
        }
    }
    if (targets.empty()) return -1;
    std::uniform_int_distribution<int> dist(0, targets.size() -1);
    return targets[dist(rng)];
}


void GameState::apply_card(int player_idx, Card card, std::mt19937& rng) {
    auto& p = players[player_idx];
    if (!p.is_active_in_round || p.is_frozen) return;

    if (card.type == CardType::NUMBER) {
        if (p.has_card_value(card.value)) {
            if (p.has_second_chance) {
                p.has_second_chance = false; // Protects once
            } else {
                p.is_active_in_round = false; // Bust
            }
        } else {
            p.active_cards.push_back(card);
            if (p.active_cards.size() == 7) {
                round_active = false; // Flip 7 ends round immediately
            }
        }
    } else if (card.type == CardType::MULTIPLIER_X2) {
        p.multiplier *= 2;
    } else if (card.type == CardType::BONUS_POINTS) {
        p.flat_bonus += card.value;
    } else if (card.type == CardType::SECOND_CHANCE) {
        if (!p.has_second_chance) p.has_second_chance = true;
        else {
            int target = get_random_active_opponent(player_idx, rng);
            if (target != -1) players[target].has_second_chance = true;
        }
    } else if (card.type == CardType::FREEZE) {
        int target = get_random_active_opponent(player_idx, rng);
        if (target != -1) players[target].is_frozen = true;
    } else if (card.type == CardType::FLIP_THREE) {
        // Scaffold: In a full sim, this triggers the recursive U_k state from your paper.
        // For basic Monte Carlo setup, we apply 3 cards sequentially to a random target.
        int target = get_random_active_opponent(player_idx, rng);
        if (target != -1) {
            for(int i=0; i<3; ++i) {
                if (!deck.is_empty()) apply_card(target, deck.draw(), rng);
            }
        }
    }
}
