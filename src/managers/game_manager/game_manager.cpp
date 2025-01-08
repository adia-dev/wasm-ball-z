#include "game_manager.hpp"
#include "entities/character/ai_character.hpp"
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

  entities::CombatStats player_stats(120, 100, 500.0f, 800.0f, 1.0f, 10, 1.1f);

  auto player =
      std::make_shared<entities::Character>(player_sprite, player_stats);

  try {
    player->animator().load_animations(utils::R::animations() + "janemba.xml");
    player->animator().play("Idle");
  } catch (const std::exception &e) {
    std::cerr << "Error loading player animations: " << e.what() << std::endl;
  }

  _game_state.entities.push_back(player);
  _game_state.player_character = player;

  Sprite computer_sprite("goku_ssjb.png", {64, 2271, 64, 64}, {0, 0, 64, 64});

  entities::CombatStats cpu_stats(100, 100, 450.0f, 750.0f, 1.2f, 12, 0.9f);

  auto computer =
      std::make_shared<entities::AICharacter>(computer_sprite, cpu_stats);
  computer->animator().load_animations(utils::R::animations() +
                                       "goku_ssjb.xml");
  computer->animator().play("Idle");
  computer->mover().set_position(Vector2f(740.0f, 400.0f));

  computer->set_opponent(player.get());

  _game_state.entities.push_back(computer);

  _game_state.player_character->stare_at(&computer->mover().position());

  _game_state.map.set_map_file("map.png");
  _game_state.map.set_map_rect({0, 3 * (1805 / 6), 1200, 1805 / 6});
}

void GameManager::update(float delta_time) {
  auto player = _game_state.player_character;
  if (!player || !player->state().is_alive()) {
    if (auto ai_opponent = std::dynamic_pointer_cast<entities::AICharacter>(
            _game_state.entities[1])) {
      _game_state.reset_episode(); // This resets episode state
      ai_opponent
          ->start_new_episode(); // This triggers the AI's episode handling
    }
    return;
  }

  if (!player->is_stunned() && !player->is_in_recovery()) {
    handle_movement_input(player);
    handle_combat_input(player);
  }

  _game_state.update_round(delta_time);

  if (!_game_state.combat_state.round_in_progress) {
    _game_state.start_new_round();
  }
}

void GameManager::handle_movement_input(
    std::shared_ptr<entities::Character> player) {

  Vector2f movement_force = Vector2f::zero();
  bool is_moving = false;

  if (InputManager::is_key_down(SDLK_LEFT) ||
      InputManager::is_key_pressed(SDLK_LEFT)) {
    movement_force = movement_force.add(Vector2f(-5000.0f, 0.0f));
    is_moving = true;
  }
  if (InputManager::is_key_down(SDLK_RIGHT) ||
      InputManager::is_key_pressed(SDLK_RIGHT)) {
    movement_force = movement_force.add(Vector2f(5000.0f, 0.0f));
    is_moving = true;
  }
  if (InputManager::is_key_down(SDLK_UP) ||
      InputManager::is_key_pressed(SDLK_UP)) {
    movement_force = movement_force.add(Vector2f(0.0f, -5000.0f));
    is_moving = true;
  }
  if (InputManager::is_key_down(SDLK_DOWN) ||
      InputManager::is_key_pressed(SDLK_DOWN)) {
    movement_force = movement_force.add(Vector2f(0.0f, 5000.0f));
    is_moving = true;
  }

  if (is_moving) {
    player->mover().add_force(movement_force);
    player->animator().play(movement_force.y < 0   ? "Up"
                            : movement_force.y > 0 ? "Down"
                            : movement_force.x < 0 ? "Left"
                                                   : "Right");
  } else {
    player->animator().play("Idle");
  }
}

void GameManager::handle_combat_input(
    std::shared_ptr<entities::Character> player) {

  if (InputManager::is_key_pressed(SDLK_z)) {
    if (player->perform_attack("light_punch")) {
      check_hit_detection(player);
    }
  }
  if (InputManager::is_key_pressed(SDLK_x)) {
    if (player->perform_attack("heavy_punch")) {
      check_hit_detection(player);
    }
  }
  if (InputManager::is_key_pressed(SDLK_c)) {
    if (player->perform_attack("light_kick")) {
      check_hit_detection(player);
    }
  }
  if (InputManager::is_key_pressed(SDLK_v)) {
    if (player->perform_attack("heavy_kick")) {
      check_hit_detection(player);
    }
  }

  if (InputManager::is_key_down(SDLK_SPACE)) {
    player->set_combat_state(entities::CombatState::BLOCKING);
  } else if (InputManager::is_key_released(SDLK_SPACE)) {
    player->set_combat_state(entities::CombatState::IDLE);
  }
}

void GameManager::check_hit_detection(
    std::shared_ptr<entities::Character> attacker) {

  for (auto &entity : _game_state.entities) {
    auto defender = std::dynamic_pointer_cast<entities::Character>(entity);
    if (defender && defender != attacker) {

      if (auto ai_defender =
              std::dynamic_pointer_cast<entities::AICharacter>(defender)) {
        ai_defender->on_got_hit();
      }
      if (auto ai_attacker =
              std::dynamic_pointer_cast<entities::AICharacter>(attacker)) {
        ai_attacker->on_hit_landed();
      }
      break;
    }
  }
}

void GameManager::update_cpu_behavior(
    std::shared_ptr<entities::Character> cpu,
    std::shared_ptr<entities::Character> player) {

  if (!cpu || !player) {
    return;
  }

  Vector2f to_player = player->mover().position().sub(cpu->mover().position());
  float distance = to_player.mag();

  if (std::isnan(distance) || distance < 0.0001f) {
    return;
  }

  Vector2f direction = to_player.normalized();
  if (std::isnan(direction.x) || std::isnan(direction.y)) {
    direction = Vector2f(1.0f, 0.0f);
  }

  if (distance > 200.0f) {
    Vector2f approach_force = direction.mul(3000.0f);
    cpu->mover().add_force(approach_force);
  } else if (distance < 100.0f) {
    Vector2f retreat_force = direction.mul(-2000.0f);
    cpu->mover().add_force(retreat_force);
  }

  const float MIN_ATTACK_DISTANCE = 20.0f;
  if (distance > MIN_ATTACK_DISTANCE && distance < 150.0f &&
      std::rand() % 100 < 5) {

    switch (std::rand() % 4) {
    case 0:
      cpu->perform_attack("light_punch");
      break;
    case 1:
      cpu->perform_attack("heavy_punch");
      break;
    case 2:
      cpu->perform_attack("light_kick");
      break;
    case 3:
      cpu->perform_attack("heavy_kick");
      break;
    }
  }
}

void GameManager::cleanup() {
  _game_state.entities.clear();
  _game_state.player_character = nullptr;
}

} // namespace managers
} // namespace wbz
