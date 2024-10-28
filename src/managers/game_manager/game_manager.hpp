#pragma once

#include <state/game_state.hpp>

namespace wbz {
namespace managers {
class GameManager {
public:
  explicit GameManager(GameState &game_state) : _game_state(game_state) {}

  void init();

  void update();
  void cleanup();

private:
  GameState &_game_state;
};
} // namespace managers
} // namespace wbz
