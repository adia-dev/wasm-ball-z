#include "game_manager.hpp"
#include "utils/r.hpp"

#include <SDL_keycode.h>
#include <entities/character/character.hpp>
#include <iostream>
#include <managers/input_manager/input_manager.hpp>
#include <memory>

namespace wbz {
namespace managers {
void GameManager::init() {
  Sprite player_sprite("janemba.png", {64, 1271, 64, 64}, {0, 0, 64, 64});
  entities::Character player(player_sprite);
  try {
    player.animator().load_animations(utils::R::animations() + "janemba.xml");
    player.animator().play("Jump");
  } catch (const std::exception &e) {
    std::cerr << "Error loading animations: " << e.what() << std::endl;
  }
  _game_state.entities.push_back(
      std::make_shared<entities::Character>(std::move(player)));

  Sprite computer_sprite("goku_ssjb.png", {64, 2271, 64, 64}, {0, 0, 64, 64});
  entities::Character computer(computer_sprite);
  computer.animator().load_animations(utils::R::animations() + "goku_ssjb.xml");
  computer.animator().play("Idle");
  computer.mover().set_position(Vector2f(740.0f, 400.0f));

  _game_state.entities.push_back(
      std::make_shared<entities::Character>(std::move(computer)));

  _game_state.player_character =
      std::dynamic_pointer_cast<entities::Character>(_game_state.entities[0]);

  std::dynamic_pointer_cast<entities::Character>(_game_state.entities[0])
      ->stare_at(&std::dynamic_pointer_cast<entities::Character>(
                      _game_state.entities[1])
                      ->mover()
                      .position());

  _game_state.map.set_map_file("map.png");
  _game_state.map.set_map_rect({0, 3 * (1805 / 6), 1200, 1805 / 6});
}
void GameManager::update() {
  _game_state.player_character->animator().play("Idle");

  if (InputManager::is_key_down(SDLK_LEFT) ||
      InputManager::is_key_pressed(SDLK_LEFT)) {
    _game_state.player_character->animator().play("Left");
    _game_state.player_character->mover().add_force({-5000.0f, 0.0f});
  }
  if (InputManager::is_key_down(SDLK_RIGHT) ||
      InputManager::is_key_pressed(SDLK_RIGHT)) {
    _game_state.player_character->animator().play("Right");
    _game_state.player_character->mover().add_force({5000.0f, 0.0f});
  }

  if (InputManager::is_key_down(SDLK_UP) ||
      InputManager::is_key_pressed(SDLK_UP)) {
    _game_state.player_character->animator().play("Up");
    _game_state.player_character->mover().add_force({0.0f, -5000.0f});
  }
  if (InputManager::is_key_down(SDLK_DOWN) ||
      InputManager::is_key_pressed(SDLK_DOWN)) {
    _game_state.player_character->animator().play("Down");
    _game_state.player_character->mover().add_force({0.0f, 5000.0f});
  }

  if (InputManager::is_key_released(SDLK_TAB)) {
    _game_state.player_character->animator().play("Jump");
  }
}
void GameManager::cleanup() {
  _game_state.entities.clear();
  _game_state.player_character = nullptr;
};
} // namespace managers
} // namespace wbz
