#include "GameState.hpp"
#include <iterator>
#include <numeric>

void PlayerState::reset_for_round() {
    active_cards.clear();
    multiplier = 1;
    flat_bonus = 0;
    has_second_chance = false;
    is_frozen = false;
    has_busted = false;
    is_active_in_round = true;
}

int PlayerState::calculate_round_score() const {
    // Only busted players score 0; players who voluntarily stayed, were frozen, or are still active score their tableau
    if (has_busted) return 0;

    int sum = 0;
    for (const auto& c : active_cards) {
        if (c.type == CardType::NUMBER) sum += c.value;
    }

    // Scoring order according to the rules: (sum * multiplier) + flat_bonus
    int score = (sum * multiplier) + flat_bonus;

    // Flip 7 check (7 distinct number cards grants +15 bonus points)
    if (active_cards.size() == 7) {
        score += 15;
    }
    return score;
}

bool PlayerState::has_card_value(int val) const {
    for (const auto& c : active_cards) {
        if (c.type == CardType::NUMBER && c.value == val) return true;
    }
    return false;
}

GameState::GameState(int num_players) {
    for (int i = 0; i < num_players; ++i) {
        PlayerState p;
        p.id = i;
        players.push_back(p);
    }
}

GameState::GameState(int num_players, std::mt19937& rng) : GameState(num_players) {
    deck.initialize();
    deck.shuffle(rng);
}

Card GameState::draw_or_reshuffle(std::mt19937& rng) {
    if (deck.is_empty()) {
        deck.initialize();
        deck.shuffle(rng);
    }
    return deck.draw();
}

void GameState::setup_round(std::mt19937& rng) {
    round_active = true;

    for (auto& p : players) {
        p.reset_for_round();
    }

    // Deal 1 card to everyone; if an action card was resolved on a player, keep dealing until everyone has >= 1 card
    for (int i = 0; i < std::ssize(players); ++i) {
        if (!round_active) break;
        if (players[i].is_active_in_round) {
            apply_card(i, draw_or_reshuffle(rng), rng);
        }
    }

    current_player_idx = 0;
}

int GameState::get_random_active_opponent(int current_idx, std::mt19937& rng, bool only_without_second_chance) const {
    std::vector<int> targets;

    for (int i = 0; i < std::ssize(players); ++i) {
        if (i != current_idx && players[i].is_active_in_round) {
            if (!only_without_second_chance || !players[i].has_second_chance) {
                targets.push_back(i);
            }
        }
    }
    if (targets.empty()) return -1;
    std::uniform_int_distribution<int> dist(0, static_cast<int>(targets.size()) - 1);
    return targets[dist(rng)];
}

void GameState::apply_card(int player_idx, Card card, std::mt19937& rng) {
    auto& p = players[player_idx];
    if (!round_active || !p.is_active_in_round || p.is_frozen) return;

    if (card.type == CardType::NUMBER) {
        if (p.has_card_value(card.value)) {
            if (p.has_second_chance) {
                p.has_second_chance = false; // Second Chance absorbs duplicate once
            } else {
                p.has_busted = true;
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
        if (!p.has_second_chance) {
            p.has_second_chance = true;
        } else {
            // Pass duplicate Second Chance only to an active opponent who doesn't already have one
            int target = get_random_active_opponent(player_idx, rng, true);
            if (target != -1) {
                players[target].has_second_chance = true;
            }
        }
    } else if (card.type == CardType::FREEZE) {
        int target = get_random_active_opponent(player_idx, rng);
        if (target == -1) target = player_idx; // Only active player left must target themselves
        players[target].is_frozen = true;
        players[target].is_active_in_round = false; // Eliminated from further actions this round
    } else if (card.type == CardType::FLIP_THREE) {
        int target = get_random_active_opponent(player_idx, rng);
        if (target == -1) target = player_idx; // Only active player left must target themselves

        for (int i = 0; i < 3; ++i) {
            // Stop immediately if the round ended (Flip 7) or the target busted/got frozen
            if (!round_active || !players[target].is_active_in_round || players[target].is_frozen) {
                break;
            }
            apply_card(target, draw_or_reshuffle(rng), rng);
        }
    }
}