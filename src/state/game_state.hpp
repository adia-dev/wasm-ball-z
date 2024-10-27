#pragma once

#include <entities/character/character.hpp>
#include <vector>

namespace wbz {
struct GameState {
  // TODO: load from file method

  std::vector<entities::Character> characters;
  entities::Character *player_character = nullptr;
};
} // namespace wbz
