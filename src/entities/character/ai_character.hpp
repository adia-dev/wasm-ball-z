#pragma once
#include "character.hpp"
#include "entities/agent/QLearningAgent.hpp"
#include <memory>

namespace wbz {
namespace entities {

class AICharacter : public Character {
public:
  AICharacter(const Sprite &sprite, const CombatStats &stats = CombatStats())
      : Character(sprite, stats),
        ai_agent(std::make_unique<ai::QLearningAgent>()),
        _previous_health(stats.max_health), _previous_opponent_health(0),
        _hit_landed(false), _got_hit(false), _time_since_last_action(0.0f),
        _training_episode(0) {
    std::cout << "\nInitializing AI Character with stats:"
              << "\nHealth: " << stats.max_health
              << "\nStamina: " << stats.max_stamina
              << "\nMovement Speed: " << stats.movement_speed
              << "\nBase Defense: " << stats.base_defense << std::endl;
  }

  void update(double delta_time) override {
    if (!_opponent) {
      Character::update(delta_time);
      return;
    }

    _time_since_last_action += delta_time;

    // Log training episode progress periodically
    if (_episode_timer <= 0.0f) {
      _training_episode++;
      std::cout << "\n=== Training Episode " << _training_episode << " ===\n"
                << "AI Health: " << state().health << "/" << state().max_health
                << "\nOpponent Health: " << _opponent->state().health << "/"
                << _opponent->state().max_health << std::endl;
      _episode_timer = 5.0f; // Reset timer for next episode log
    }
    _episode_timer -= delta_time;

    // Get current state
    auto current_state = ai_agent->get_state(
        mover().position(), _opponent->mover().position(),
        _opponent->get_combat_state() == CombatState::ATTACKING,
        static_cast<float>(state().health) / state().max_health);

    // Calculate reward based on what happened since last update
    float health_change = _previous_health - state().health;
    float opponent_health_change =
        _previous_opponent_health - _opponent->state().health;

    float distance =
        Vector2f(mover().position().sub(_opponent->mover().position())).mag();

    float reward = ai_agent->calculate_reward(
        health_change, opponent_health_change, distance, _hit_landed, _got_hit,
        _time_since_last_action);

    // Update Q-values if we have a previous state and action
    if (_has_previous_state) {
      ai_agent->update(_previous_state, _previous_action, reward,
                       current_state);
    }

    // Select and execute new action
    auto action = ai_agent->select_action(current_state);
    execute_action(action);

    // Store current state for next update
    _previous_state = current_state;
    _previous_action = action;
    _has_previous_state = true;

    // Store current health values
    _previous_health = state().health;
    _previous_opponent_health = _opponent->state().health;

    // Reset hit flags
    _hit_landed = false;
    _got_hit = false;

    // Decay exploration rate
    ai_agent->decay_exploration();

    // Call parent update
    Character::update(delta_time);
  }

  void set_opponent(Character *opponent) {
    _opponent = opponent;
    _previous_opponent_health = opponent->state().health;
  }

  void on_hit_landed() { _hit_landed = true; }

  void on_got_hit() { _got_hit = true; }

private:
  std::unique_ptr<ai::QLearningAgent> ai_agent;
  Character *_opponent = nullptr;

  // State tracking
  ai::State _previous_state;
  ai::Action _previous_action;
  bool _has_previous_state = false;

  // Stats tracking
  float _previous_health;
  float _previous_opponent_health;
  float _episode_timer;
  float _time_since_last_action;
  int _training_episode;
  bool _hit_landed;
  bool _got_hit;

  void execute_action(ai::Action action) {
    const float MOVEMENT_FORCE = 5000.0f;

    switch (action) {
    case ai::Action::MOVE_LEFT:
      mover().add_force(Vector2f(-MOVEMENT_FORCE, 0.0f));
      break;

    case ai::Action::MOVE_RIGHT:
      mover().add_force(Vector2f(MOVEMENT_FORCE, 0.0f));
      break;

    case ai::Action::MOVE_UP:
      mover().add_force(Vector2f(0.0f, -MOVEMENT_FORCE));
      break;

    case ai::Action::MOVE_DOWN:
      mover().add_force(Vector2f(0.0f, MOVEMENT_FORCE));
      break;

    case ai::Action::LIGHT_PUNCH:
      perform_attack("light_punch");
      break;

    case ai::Action::HEAVY_PUNCH:
      perform_attack("heavy_punch");
      break;

    case ai::Action::LIGHT_KICK:
      perform_attack("light_kick");
      break;

    case ai::Action::HEAVY_KICK:
      perform_attack("heavy_kick");
      break;

    case ai::Action::BLOCK:
      set_combat_state(CombatState::BLOCKING);
      break;

    case ai::Action::IDLE:
      set_combat_state(CombatState::IDLE);
      break;
    }
  }
};

} // namespace entities
} // namespace wbz
