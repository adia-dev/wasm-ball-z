#include "game_manager.hpp"

#include <SDL_keycode.h>
#include <entities/character/character.hpp>
#include <managers/input_manager/input_manager.hpp>
#include <memory>

namespace wbz {
namespace managers {
void GameManager::init() {
  Sprite player_sprite("janemba.png", {64, 1271, 64, 64}, {0, 0, 64, 64});
  entities::Character player(player_sprite);
  player.set_width(30);
  player.set_height(60);
  player.mover().set_position(Vector2f(50.0f, 400.0f));
  player.animator().add_animation(
      "jump", {
                  .delay = 100,
                  .frames =
                      {
                          {.x = 262, .y = 617, .w = 49, .h = 42},
                          {.x = 332, .y = 575, .w = 28, .h = 58},
                          {.x = 381, .y = 559, .w = 46, .h = 46},
                          {.x = 442, .y = 583, .w = 43, .h = 55},
                          {.x = 502, .y = 617, .w = 49, .h = 42},
                      },
              });
  player.animator().play("jump");
  _game_state.entities.push_back(
      std::make_shared<entities::Character>(std::move(player)));

  Sprite computer_sprite("janemba.png", {64, 2271, 64, 64}, {0, 0, 64, 64});
  entities::Character computer(computer_sprite);
  computer.mover().set_position(Vector2f(740.0f, 400.0f));
  computer.set_width(20);
  computer.set_height(40);
  computer.animator().add_animation(
      "jump", {
                  .delay = 100,
                  .frames =
                      {
                          {.x = 262, .y = 617, .w = 49, .h = 42},
                          {.x = 332, .y = 575, .w = 28, .h = 58},
                          {.x = 381, .y = 559, .w = 46, .h = 46},
                          {.x = 442, .y = 583, .w = 43, .h = 55},
                          {.x = 502, .y = 617, .w = 49, .h = 42},
                      },
              });
  computer.animator().play("jump");
  _game_state.entities.push_back(
      std::make_shared<entities::Character>(std::move(computer)));

  _game_state.player_character =
      std::dynamic_pointer_cast<entities::Character>(_game_state.entities[0]);

  std::dynamic_pointer_cast<entities::Character>(_game_state.entities[0])
      ->stare_at(&std::dynamic_pointer_cast<entities::Character>(
                      _game_state.entities[1])
                      ->mover()
                      .position());
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
  _game_state.entities.clear();
  _game_state.player_character = nullptr;
};
} // namespace managers
} // namespace wbz
