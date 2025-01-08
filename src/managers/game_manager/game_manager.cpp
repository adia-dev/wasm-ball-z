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
  // Create the player character with combat-ready configuration
  Sprite player_sprite("janemba.png", {64, 1271, 64, 64}, {0, 0, 64, 64});

  // Define custom combat stats for player
  entities::CombatStats player_stats(
      120,    // health
      100,    // stamina
      500.0f, // movement speed
      800.0f, // jump force
      1.0f,   // weight
      10,     // base defense
      1.1f    // attack speed modifier (slightly faster)
  );

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

  // Create the computer opponent with different stats
  Sprite computer_sprite("goku_ssjb.png", {64, 2271, 64, 64}, {0, 0, 64, 64});

  // Define custom combat stats for CPU
  entities::CombatStats cpu_stats(
      100,    // health
      100,    // stamina
      450.0f, // movement speed
      750.0f, // jump force
      1.2f,   // weight (slightly heavier)
      12,     // base defense (more defensive)
      0.9f    // attack speed modifier (slightly slower)
  );

  auto computer =
      std::make_shared<entities::Character>(computer_sprite, cpu_stats);
  computer->animator().load_animations(utils::R::animations() +
                                       "goku_ssjb.xml");
  computer->animator().play("Idle");
  computer->mover().set_position(Vector2f(740.0f, 400.0f));

  _game_state.entities.push_back(computer);

  // Set up character facing
  _game_state.player_character->stare_at(&computer->mover().position());

  // Initialize the stage
  _game_state.map.set_map_file("map.png");
  _game_state.map.set_map_rect({0, 3 * (1805 / 6), 1200, 1805 / 6});
}

void GameManager::update() {
  auto player = _game_state.player_character;
  if (!player || !player->state().is_alive()) {
    return;
  }

  // Only process inputs if the player isn't stunned or in recovery
  if (!player->is_stunned() && !player->is_in_recovery()) {
    handle_movement_input(player);
    handle_combat_input(player);
  }

  // Get the CPU opponent (assumes it's the second entity)
  auto cpu =
      std::dynamic_pointer_cast<entities::Character>(_game_state.entities[1]);
  if (cpu && cpu->state().is_alive()) {
    update_cpu_behavior(cpu, player);
  }
}

void GameManager::handle_movement_input(
    std::shared_ptr<entities::Character> player) {
  // Base movement force
  Vector2f movement_force = Vector2f::zero();
  bool is_moving = false;

  // Movement controls
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

  // Apply movement and update animation
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
  // Attack inputs
  if (InputManager::is_key_pressed(SDLK_z)) {
    player->perform_attack("light_punch");
  }
  if (InputManager::is_key_pressed(SDLK_x)) {
    player->perform_attack("heavy_punch");
  }
  if (InputManager::is_key_pressed(SDLK_c)) {
    player->perform_attack("light_kick");
  }
  if (InputManager::is_key_pressed(SDLK_v)) {
    player->perform_attack("heavy_kick");
  }

  // Blocking
  if (InputManager::is_key_down(SDLK_SPACE)) {
    player->set_combat_state(entities::CombatState::BLOCKING);
  } else if (InputManager::is_key_released(SDLK_SPACE)) {
    player->set_combat_state(entities::CombatState::IDLE);
  }
}

void GameManager::update_cpu_behavior(
    std::shared_ptr<entities::Character> cpu,
    std::shared_ptr<entities::Character> player) {
  // Validate pointers
  if (!cpu || !player) {
    return;
  }

  // Get the distance vector to player with validation
  Vector2f to_player = player->mover().position().sub(cpu->mover().position());
  float distance = to_player.mag();

  // Protect against NaN or invalid distances
  if (std::isnan(distance) || distance < 0.0001f) {
    return; // Skip AI update if too close or invalid
  }

  // Normalize direction safely
  Vector2f direction = to_player.normalized();
  if (std::isnan(direction.x) || std::isnan(direction.y)) {
    direction =
        Vector2f(1.0f, 0.0f); // Default direction if normalization fails
  }

  // AI behavior with safe force calculations
  if (distance > 200.0f) {
    Vector2f approach_force = direction.mul(3000.0f);
    cpu->mover().add_force(approach_force);
  } else if (distance < 100.0f) {
    Vector2f retreat_force = direction.mul(-2000.0f);
    cpu->mover().add_force(retreat_force);
  }

  // Safe attack distance check
  const float MIN_ATTACK_DISTANCE = 20.0f; // Minimum safe distance for attacks
  if (distance > MIN_ATTACK_DISTANCE && distance < 150.0f &&
      std::rand() % 100 < 5) {
    // Attack logic remains the same but only executes at safe distances
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
