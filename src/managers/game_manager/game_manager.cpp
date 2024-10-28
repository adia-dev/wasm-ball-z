#include "game_manager.hpp"

#include <SDL_keycode.h>
#include <managers/input_manager/input_manager.hpp>

namespace wbz {
namespace managers {
void GameManager::init() {
  entities::Character player("adia-dev");
  player.set_width(30);
  player.set_height(60);
  player.mover().set_position(Vector2f(50.0f, 400.0f));
  _game_state.characters.push_back(player);

  entities::Character computer("<COMPUTER>");
  computer.mover().set_position(Vector2f(740.0f, 400.0f));
  computer.set_width(20);
  computer.set_height(40);
  _game_state.characters.push_back(std::move(computer));

  _game_state.player_character = &_game_state.characters[0];
}
void GameManager::update() {
  if (InputManager::is_key_down(SDLK_LEFT) ||
      InputManager::is_key_pressed(SDLK_LEFT)) {
    _game_state.player_character->mover().add_force({-5000.0f, 0.0f});
  }
  if (InputManager::is_key_down(SDLK_RIGHT) ||
      InputManager::is_key_pressed(SDLK_RIGHT)) {
    _game_state.player_character->mover().add_force({5000.0f, 0.0f});
  }

  if (InputManager::is_key_down(SDLK_UP) ||
      InputManager::is_key_pressed(SDLK_UP)) {
    _game_state.player_character->mover().add_force({0.0f, -5000.0f});
  }
  if (InputManager::is_key_down(SDLK_DOWN) ||
      InputManager::is_key_pressed(SDLK_DOWN)) {
    _game_state.player_character->mover().add_force({0.0f, 5000.0f});
  }
}
void GameManager::cleanup() {
  _game_state.characters.clear();
  _game_state.player_character = nullptr;
};
} // namespace managers
} // namespace wbz
