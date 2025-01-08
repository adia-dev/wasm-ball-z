#pragma once

#include "entities/entity.hpp"
#include "map/map.hpp"
#include <entities/character/character.hpp>
#include <memory>
#include <vector>

namespace wbz {

// Additional state for tracking combat
struct CombatRoundState {
  float round_timer = 99.0f; // Traditional fighting game countdown
  int round_number = 1;
  int max_rounds = 3;
  int player_rounds_won = 0;
  int opponent_rounds_won = 0;
  bool round_in_progress = true;
};

struct GameState {
  // Core game elements
  Map map;
  std::vector<std::shared_ptr<entities::Entity>> entities;
  std::shared_ptr<entities::Character> player_character = nullptr;

  // Combat specific state
  CombatRoundState combat_state;

  // Round management
  void start_new_round() {
    combat_state.round_timer = 99.0f;
    combat_state.round_in_progress = true;

    // Reset character positions and states
    if (player_character) {
      player_character->mover().set_position(Vector2f(200.0f, 400.0f));
      player_character->state().heal(player_character->state().max_health);
    }

    // Reset opponent if it exists (assuming it's the second entity)
    if (entities.size() > 1) {
      auto opponent =
          std::dynamic_pointer_cast<entities::Character>(entities[1]);
      if (opponent) {
        opponent->mover().set_position(Vector2f(600.0f, 400.0f));
        opponent->state().heal(opponent->state().max_health);
      }
    }
  }

  void update_round(float delta_time) {
    if (!combat_state.round_in_progress) {
      return;
    }

    // Update timer
    combat_state.round_timer -= delta_time;

    // Check for round end conditions
    bool round_ended = false;

    // Time out
    if (combat_state.round_timer <= 0.0f) {
      round_ended = true;
      // Determine winner based on remaining health percentage
      if (player_character && entities.size() > 1) {
        auto opponent =
            std::dynamic_pointer_cast<entities::Character>(entities[1]);
        if (opponent) {
          float player_health_percent =
              static_cast<float>(player_character->state().health) /
              player_character->state().max_health;
          float opponent_health_percent =
              static_cast<float>(opponent->state().health) /
              opponent->state().max_health;

          if (player_health_percent > opponent_health_percent) {
            combat_state.player_rounds_won++;
          } else if (opponent_health_percent > player_health_percent) {
            combat_state.opponent_rounds_won++;
          }
          // In case of exact tie, both get a point
        }
      }
    }

    // KO
    if (player_character && !player_character->state().is_alive()) {
      round_ended = true;
      combat_state.opponent_rounds_won++;
    } else if (entities.size() > 1) {
      auto opponent =
          std::dynamic_pointer_cast<entities::Character>(entities[1]);
      if (opponent && !opponent->state().is_alive()) {
        round_ended = true;
        combat_state.player_rounds_won++;
      }
    }

    // Handle round end
    if (round_ended) {
      combat_state.round_in_progress = false;
      combat_state.round_number++;

      // Check if match is over
      if (combat_state.round_number <= combat_state.max_rounds &&
          combat_state.player_rounds_won < (combat_state.max_rounds / 2 + 1) &&
          combat_state.opponent_rounds_won <
              (combat_state.max_rounds / 2 + 1)) {
        // Start next round after a delay (handled in game manager)
        start_new_round();
      }
    }
  }
};

} // namespace wbz
