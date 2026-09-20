import random
from functools import lru_cache

# Initial 79-card deck based on the manual: 1x0, 1x1, 2x2... 12x12
INITIAL_DECK = (1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)
WIN_SCORE = 200

# ==========================================
# 1. MDP Value Functions (The "Brains")
# ==========================================

@lru_cache(maxsize=None)
def V2(C, D):
    """Expected score for the player acting second in a round."""
    deck_size = sum(D)
    stop_value = sum(C)
    if deck_size == 0: return stop_value
        
    draw_value = 0
    for x in range(13):
        if D[x] > 0:
            prob = D[x] / deck_size
            new_D = tuple(D[i] - (1 if i == x else 0) for i in range(13))
            
            if x in C:
                draw_value += prob * 0
            elif len(C) == 6:
                draw_value += prob * (sum(C) + x + 15)
            else:
                draw_value += prob * V2(C.union({x}), new_D)
                
    return max(stop_value, draw_value)

@lru_cache(maxsize=None)
def V1(C, D):
    """Expected point differential for the player acting first in a round."""
    deck_size = sum(D)
    stop_value = sum(C) - V2(frozenset(), D)
    if deck_size == 0: return stop_value
        
    draw_value = 0
    for x in range(13):
        if D[x] > 0:
            prob = D[x] / deck_size
            new_D = tuple(D[i] - (1 if i == x else 0) for i in range(13))
            
            if x in C:
                draw_value += prob * (0 - V2(frozenset(), new_D))
            elif len(C) == 6:
                draw_value += prob * ((sum(C) + x + 15) - V2(frozenset(), new_D))
            else:
                draw_value += prob * V1(C.union({x}), new_D)
                
    return max(stop_value, draw_value)

# ==========================================
# 2. Policy Extraction (Making the Choice)
# ==========================================

def get_action_p2(C, D):
    """Determines whether the second player should STOPP or NOCH EINE."""
    stop_val = sum(C)
    deck_size = sum(D)
    if deck_size == 0: return "STOPP"
    
    draw_val = 0
    for x in range(13):
        if D[x] > 0:
            prob = D[x] / deck_size
            new_D = tuple(D[i] - (1 if i == x else 0) for i in range(13))
            if x in C: draw_val += prob * 0
            elif len(C) == 6: draw_val += prob * (sum(C) + x + 15)
            else: draw_val += prob * V2(C.union({x}), new_D)
            
    return "STOPP" if stop_val >= draw_val else "NOCH EINE"

def get_action_p1(C, D):
    """Determines whether the first player should STOPP or NOCH EINE."""
    stop_val = sum(C) - V2(frozenset(), D)
    deck_size = sum(D)
    if deck_size == 0: return "STOPP"
    
    draw_val = 0
    for x in range(13):
        if D[x] > 0:
            prob = D[x] / deck_size
            new_D = tuple(D[i] - (1 if i == x else 0) for i in range(13))
            if x in C: draw_val += prob * (0 - V2(frozenset(), new_D))
            elif len(C) == 6: draw_val += prob * ((sum(C) + x + 15) - V2(frozenset(), new_D))
            else: draw_val += prob * V1(C.union({x}), new_D)
            
    return "STOPP" if stop_val >= draw_val else "NOCH EINE"

# ==========================================
# 3. Game Engine Mechanics
# ==========================================

def draw_card(D):
    """Randomly draws a card based on current deck probabilities."""
    deck_size = sum(D)
    if deck_size == 0:
        D = INITIAL_DECK # Reshuffle if empty
        deck_size = sum(D)
        
    rand = random.uniform(0, 1)
    cumulative = 0
    for x in range(13):
        if D[x] > 0:
            cumulative += D[x] / deck_size
            if rand <= cumulative:
                new_D = tuple(D[i] - (1 if i == x else 0) for i in range(13))
                return x, new_D
    return 12, D # Fallback

def play_turn(is_first_player_of_round, starting_D):
    """Simulates a single player's turn."""
    C = set()
    D = starting_D
    
    # 1. Rundenvorbereitung: Jeder bekommt mindestens 1 Karte
    first_card, D = draw_card(D)
    C.add(first_card)
    
    while True:
        # Ask policy what to do
        action = get_action_p1(frozenset(C), D) if is_first_player_of_round else get_action_p2(frozenset(C), D)
        
        if action == "STOPP":
            return sum(C), D
            
        # Action is "NOCH EINE"
        drawn_card, D = draw_card(D)
        
        if drawn_card in C:
            return 0, D # Busted (Verzockt)
        elif len(C) == 6:
            C.add(drawn_card)
            return sum(C) + 15, D # Flip 7!
        else:
            C.add(drawn_card)

# ==========================================
# 4. Simulation Loop
# ==========================================

def simulate_game(starting_player=1):
    scores = {1: 0, 2: 0}
    current_deck = INITIAL_DECK
    
    first_actor = starting_player
    
    while True:
        second_actor = 2 if first_actor == 1 else 1
        
        # Reshuffle if deck is empty before round starts
        if sum(current_deck) == 0:
            current_deck = INITIAL_DECK
            
        # Play First Actor
        points_1, current_deck = play_turn(is_first_player_of_round=True, starting_D=current_deck)
        scores[first_actor] += points_1
        
        # Play Second Actor
        points_2, current_deck = play_turn(is_first_player_of_round=False, starting_D=current_deck)
        scores[second_actor] += points_2
        
        # Check Win Conditions
        if scores[1] >= WIN_SCORE or scores[2] >= WIN_SCORE:
            if scores[1] != scores[2]: # Resolve ties
                break
                
        # Bisheriger Startspieler reicht Reststapel nach links an den neuen Startspieler
        first_actor = second_actor 
        
    return 1 if scores[1] > scores[2] else 2

def run_monte_carlo(iterations=10000):
    print("Pre-calculating DP tables and running simulation. This takes a moment on the first run...")
    wins_p1_started = {1: 0, 2: 0}
    
    # Simulate games where Player 1 starts the entire game
    for _ in range(iterations):
        print(_)
        if _ % 10 == 0:
            print("Hello world")
            
        winner = simulate_game(starting_player=1)
        wins_p1_started[winner] += 1
        
    p1_win_rate = (wins_p1_started[1] / iterations) * 100
    p2_win_rate = (wins_p1_started[2] / iterations) * 100
    
    print(f"\n--- Results after {iterations} games (Player 1 starts first round) ---")
    print(f"Player 1 Win Probability: {p1_win_rate:.2f}%")
    print(f"Player 2 Win Probability: {p2_win_rate:.2f}%")

if __name__ == "__main__":
    # You can lower the iterations to 1,000 if you want it to run faster initially.
    run_monte_carlo(1000)