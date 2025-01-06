#pragma once

#include "entities/entity.hpp"
#include "map/map.hpp"
#include <entities/character/character.hpp>
#include <vector>

namespace wbz {
struct GameState {
  // TODO: load from file method

  Map map;
  std::vector<std::shared_ptr<entities::Entity>> entities;
  std::shared_ptr<entities::Character> player_character = nullptr;
};
} // namespace wbz
