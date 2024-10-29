#pragma once

#include "entities/entity.hpp"
#include <entities/character/character.hpp>
#include <vector>

namespace wbz {
struct GameState {
  // TODO: load from file method

  std::vector<std::shared_ptr<entities::Entity>> entities;
  std::shared_ptr<entities::Character> player_character = nullptr;
};
} // namespace wbz
