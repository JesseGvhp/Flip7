#include "GameState.hpp"
#include "Policy.hpp"
#include <algorithm>
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
    if (has_busted) return 0;

    int sum = 0;
    for (const auto& c : active_cards) {
        if (c.type == CardType::NUMBER) sum += c.value;
    }

    int score = (sum * multiplier) + flat_bonus;

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

std::vector<int> GameState::get_valid_targets(int acting_idx, CardType card_type) const {
    std::vector<int> targets;
    const int n = static_cast<int>(players.size());

    if (card_type == CardType::SECOND_CHANCE) {
        // Duplicate Second Chance must go to another active player who doesn't have one
        for (int i = 0; i < n; ++i) {
            if (i != acting_idx && players[i].is_active_in_round && !players[i].has_second_chance) {
                targets.push_back(i);
            }
        }
    } else {
        // FREEZE and FLIP_THREE can target any active player
        for (int i = 0; i < n; ++i) {
            if (players[i].is_active_in_round) {
                targets.push_back(i);
            }
        }
    }
    return targets;
}

int GameState::get_random_active_opponent(int current_idx, std::mt19937& rng, bool only_without_second_chance) const {
    std::vector<int> targets;
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
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

int GameState::select_target(
    int acting_idx,
    CardType card_type,
    std::mt19937& rng,
    const std::vector<const Policy*>* policies) const 
{
    if (policies && acting_idx >= 0 && acting_idx < static_cast<int>(policies->size()) && (*policies)[acting_idx]) {
        std::vector<int> valid = get_valid_targets(acting_idx, card_type);
        if (valid.empty()) return -1;
        int chosen = (*policies)[acting_idx]->choose_action_target(*this, acting_idx, card_type, valid);
        if (std::find(valid.begin(), valid.end(), chosen) != valid.end()) {
            return chosen;
        }
        return valid.front();
    }

    // Fallback when no policies are passed (e.g., existing unit tests)
    if (card_type == CardType::SECOND_CHANCE) {
        return get_random_active_opponent(acting_idx, rng, true);
    }
    int target = get_random_active_opponent(acting_idx, rng, false);
    if (target == -1 && players[acting_idx].is_active_in_round) {
        target = acting_idx;
    }
    return target;
}

void GameState::setup_round(std::mt19937& rng, const std::vector<const Policy*>* policies) {
    round_active = true;

    for (auto& p : players) {
        p.reset_for_round();
    }

    const int n = static_cast<int>(players.size());
    for (int offset = 0; offset < n; ++offset) {
        if (!round_active) break;
        int idx = (starting_player_idx + offset) % n;
        if (players[idx].is_active_in_round) {
            apply_card(idx, draw_or_reshuffle(rng), rng, policies);
        }
    }

    current_player_idx = starting_player_idx;
}

void GameState::apply_card(
    int player_idx,
    Card card,
    std::mt19937& rng,
    const std::vector<const Policy*>* policies) 
{
    auto& p = players[player_idx];
    if (!round_active || !p.is_active_in_round || p.is_frozen) return;

    if (card.type == CardType::NUMBER) {
        if (p.has_card_value(card.value)) {
            if (p.has_second_chance) {
                p.has_second_chance = false;
            } else {
                p.has_busted = true;
                p.is_active_in_round = false;
            }
        } else {
            p.active_cards.push_back(card);
            if (p.active_cards.size() == 7) {
                round_active = false;
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
            int target = select_target(player_idx, CardType::SECOND_CHANCE, rng, policies);
            if (target != -1) {
                players[target].has_second_chance = true;
            }
        }
    } else if (card.type == CardType::FREEZE) {
        int target = select_target(player_idx, CardType::FREEZE, rng, policies);
        if (target != -1) {
            players[target].is_frozen = true;
            players[target].is_active_in_round = false;
        }
    } else if (card.type == CardType::FLIP_THREE) {
        int target = select_target(player_idx, CardType::FLIP_THREE, rng, policies);
        if (target == -1) return;

        // Per the rules: target draws up to 3 cards one by one.
        // Action cards drawn during Flip Three are queued and resolved AFTER the 3 draws finish
        // (even if the target busts on a subsequent draw among the 3).
        std::vector<Card> queued_actions;
        for (int i = 0; i < 3; ++i) {
            if (!round_active || !players[target].is_active_in_round || players[target].is_frozen) {
                break;
            }
            Card drawn = draw_or_reshuffle(rng);
            if (drawn.type == CardType::FREEZE ||
                drawn.type == CardType::SECOND_CHANCE ||
                drawn.type == CardType::FLIP_THREE) {
                queued_actions.push_back(drawn);
            } else {
                apply_card(target, drawn, rng, policies);
            }
        }

        // Resolve queued action cards. If target busted, they must pass SECOND_CHANCE as well.
        for (const auto& action_card : queued_actions) {
            if (!round_active || !is_any_player_active()) break;

            if (action_card.type == CardType::SECOND_CHANCE) {
                if (players[target].is_active_in_round && !players[target].has_second_chance) {
                    players[target].has_second_chance = true;
                } else {
                    int sc_target = select_target(target, CardType::SECOND_CHANCE, rng, policies);
                    if (sc_target != -1) {
                        players[sc_target].has_second_chance = true;
                    }
                }
            } else if (action_card.type == CardType::FREEZE) {
                int freeze_target = select_target(target, CardType::FREEZE, rng, policies);
                if (freeze_target != -1) {
                    players[freeze_target].is_frozen = true;
                    players[freeze_target].is_active_in_round = false;
                }
            } else if (action_card.type == CardType::FLIP_THREE) {
                int f3_target = select_target(target, CardType::FLIP_THREE, rng, policies);
                if (f3_target != -1) {
                    // Temporarily allow resolution on f3_target
                    apply_card(f3_target, action_card, rng, policies);
                }
            }
        }
    }
}

bool GameState::is_any_player_active() const {
    for (const auto& p : players) {
        if (p.is_active_in_round) return true;
    }
    return false;
}

void GameState::play_round(std::mt19937& rng, const std::vector<const Policy*>& policies) {
    setup_round(rng, &policies);

    const int n = static_cast<int>(players.size());
    while (round_active && is_any_player_active()) {
        auto& p = players[current_player_idx];
        if (p.is_active_in_round) {
            if (policies[current_player_idx]->should_draw(*this, current_player_idx)) {
                apply_card(current_player_idx, draw_or_reshuffle(rng), rng, &policies);
            } else {
                p.is_active_in_round = false; // Voluntarily stayed ("STOPP")
            }
        }
        current_player_idx = (current_player_idx + 1) % n;
    }

    round_active = false;
    for (auto& p : players) {
        p.total_score += p.calculate_round_score();
    }

    // Pass remaining deck to the left for the next round
    starting_player_idx = (starting_player_idx + 1) % n;
}

int GameState::get_winner(int win_score) const {
    int max_score = -1;
    int max_count = 0;
    int winner_idx = -1;

    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        int s = players[i].total_score;
        if (s > max_score) {
            max_score = s;
            max_count = 1;
            winner_idx = i;
        } else if (s == max_score) {
            max_count++;
        }
    }

    // Game ends when at least one player reaches >= win_score AND there is no tie for 1st
    if (max_score >= win_score && max_count == 1) {
        return winner_idx;
    }
    return -1;
}