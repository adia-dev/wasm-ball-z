#pragma once

#include "entities/entity.hpp"
#include "map/map.hpp"
#include <entities/character/character.hpp>
#include <memory>
#include <vector>

namespace wbz {

struct EpisodeState {
  int episode_number;
  float episode_timer;
  float total_episode_time;
  int hits_landed;
  int hits_taken;
  float average_distance;
  float total_damage_dealt;
  float total_damage_taken;

  EpisodeState()
      : episode_number(0), episode_timer(0), total_episode_time(0),
        hits_landed(0), hits_taken(0), average_distance(0),
        total_damage_dealt(0), total_damage_taken(0) {}

  void reset() {
    episode_timer = 0;
    total_episode_time = 0;
    hits_landed = 0;
    hits_taken = 0;
    average_distance = 0;
    total_damage_dealt = 0;
    total_damage_taken = 0;
  }
};

struct CombatRoundState {
  float round_timer = 99.0f;
  int round_number = 1;
  int max_rounds = 3;
  int player_rounds_won = 0;
  int opponent_rounds_won = 0;
  bool round_in_progress = true;
};

struct GameState {

  Map map;
  std::vector<std::shared_ptr<entities::Entity>> entities;
  std::shared_ptr<entities::Character> player_character = nullptr;

  CombatRoundState combat_state;
  EpisodeState episode_state;

  void reset_episode() {
    episode_state.episode_number++;
    episode_state.reset();

    // Reset character positions and health
    if (player_character) {
      player_character->reset();
      player_character->mover().set_position(Vector2f(200.0f, 400.0f));
    }

    if (entities.size() > 1) {
      auto opponent =
          std::dynamic_pointer_cast<entities::Character>(entities[1]);
      if (opponent) {
        opponent->reset();
        opponent->mover().set_position(Vector2f(600.0f, 400.0f));
      }
    }

    combat_state.round_timer = 99.0f;
    combat_state.round_in_progress = true;
  }

  void start_new_round() {
    combat_state.round_timer = 99.0f;
    combat_state.round_in_progress = true;

    if (player_character) {
      player_character->mover().set_position(Vector2f(200.0f, 400.0f));
      player_character->state().heal(player_character->state().max_health);
    }

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

    combat_state.round_timer -= delta_time;

    bool round_ended = false;

    if (combat_state.round_timer <= 0.0f) {
      round_ended = true;

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
        }
      }
    }

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

    if (round_ended) {
      combat_state.round_in_progress = false;
      combat_state.round_number++;

      if (combat_state.round_number <= combat_state.max_rounds &&
          combat_state.player_rounds_won < (combat_state.max_rounds / 2 + 1) &&
          combat_state.opponent_rounds_won <
              (combat_state.max_rounds / 2 + 1)) {

        start_new_round();
      }
    }
  }
};

} // namespace wbz
