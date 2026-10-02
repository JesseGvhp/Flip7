#pragma once
#include <string>

enum class CardType{
    NUMBER,
    MULTIPLIER_X2,
    BONUS_POINTS,
    FREEZE,
    SECOND_CHANCE,
    FLIP_THREE
};

struct Card {
    CardType type;
    int value; // This is the face vlaue for NUMBER or BONUS_POINTS, 0 for actions/multipliers

    bool operator ==(const Card& other) const{
        return type == other.type && value == other.value;
    }
};