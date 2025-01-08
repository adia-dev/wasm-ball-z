#pragma once

#include <entities/character/character.hpp>
#include <memory>
#include <state/game_state.hpp>

namespace wbz {
namespace managers {
class GameManager {
public:
  explicit GameManager(GameState &game_state) : _game_state(game_state) {}

  void init();
  void update(float delta_time);
  void cleanup();

private:
  GameState &_game_state;

  // New helper methods for handling different aspects of the game
  void handle_movement_input(std::shared_ptr<entities::Character> player);
  void handle_combat_input(std::shared_ptr<entities::Character> player);
  void update_cpu_behavior(std::shared_ptr<entities::Character> cpu,
                           std::shared_ptr<entities::Character> player);
  void check_hit_detection(std::shared_ptr<entities::Character> attacker);
};
} // namespace managers
} // namespace wbz
