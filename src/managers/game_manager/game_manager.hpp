#pragma once

#include <state/game_state.hpp>

#include "SDL_keycode.h"
#include "managers/input_manager/input_manager.hpp"

namespace wbz {
namespace managers {
class GameManager {
public:
  explicit GameManager(GameState &game_state) : _game_state(game_state) {}

  void init() {
    entities::Character player("adia-dev");
    player.mover().set_position(Vector2f(50.0f, 400.0f));
    _game_state.characters.push_back(player);

    entities::Character computer("<COMPUTER>");
    computer.mover().set_position(Vector2f(740.0f, 400.0f));
    _game_state.characters.push_back(std::move(computer));

    _game_state.player_character = &_game_state.characters[0];
  }

  void update() {
    if (InputManager::is_key_down(SDLK_LEFT)) {
      _game_state.player_character->mover().add_force({-5000.0f, 0.0f});
    }
    if (InputManager::is_key_down(SDLK_RIGHT)) {
      _game_state.player_character->mover().add_force({5000.0f, 0.0f});
    }

    if (InputManager::is_key_down(SDLK_UP)) {
      _game_state.player_character->mover().add_force({0.0f, -5000.0f});
    }
    if (InputManager::is_key_down(SDLK_DOWN)) {
      _game_state.player_character->mover().add_force({0.0f, 5000.0f});
    }
  }

  void cleanup() {
    _game_state.characters.clear();
    _game_state.player_character = nullptr;
  };

private:
  GameState &_game_state;
};
} // namespace managers
} // namespace wbz
