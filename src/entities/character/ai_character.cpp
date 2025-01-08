#include "ai_character.hpp"
#include <cmath>
#include <iostream>

namespace wbz {
namespace entities {

void AICharacter::update(double delta_time) {

  if (!_opponent) {
    Character::update(delta_time);
    return;
  }

  _time_since_last_action += static_cast<float>(delta_time);

  bool in_radar = is_opponent_in_radar();

  // Radar check: if opponent is not in radar, expand the radar
  if (!in_radar) {
    // Expand the radar
    if (_radar_radius < _max_radar_radius) {
      _radar_radius += _radar_expand_speed * static_cast<float>(delta_time);
      std::cout << "[Episode: " << _training_episode
                << "][Radar] Expanding radius -> " << _radar_radius
                << std::endl;
    }
  } else {

    _radar_radius = std::max(_radar_radius - 10.0f * (float)delta_time, 50.0f);
    std::cout << "[Episode: " << _training_episode
              << "][Radar] Opponent found! Radar shrinking -> " << _radar_radius
              << std::endl;
  }

  if (_episode_timer <= 0.0f) {
    _training_episode++;
    std::cout << "\n=== Training Episode " << _training_episode << " ===\n"
              << "AI Health: " << state().health << "/" << state().max_health
              << "\nOpponent Health: " << _opponent->state().health << "/"
              << _opponent->state().max_health << std::endl;
    _episode_timer = 5.0f;
  }
  _episode_timer -= static_cast<float>(delta_time);

  auto current_state = ai_agent->get_state(
      mover().position(), _opponent->mover().position(),
      _opponent->get_combat_state() == CombatState::ATTACKING,
      static_cast<float>(state().health) / state().max_health);

  current_state.opponent_in_radar = in_radar;

  float health_change = _previous_health - state().health;
  float opponent_health_change =
      _previous_opponent_health - _opponent->state().health;
  float distance = mover().position().sub(_opponent->mover().position()).mag();

  _average_distance = (_average_distance * 0.95f) + (distance * 0.05f);

  float reward = ai_agent->calculate_reward(
      health_change, opponent_health_change, distance, _hit_landed, _got_hit,
      _time_since_last_action, in_radar);

  if (_has_previous_state) {
    ai_agent->update(_previous_state, _previous_action, reward, current_state);
  }

  auto action = ai_agent->select_action(current_state);
  execute_action(action);

  _previous_state = current_state;
  _previous_action = action;
  _has_previous_state = true;
  _previous_health = state().health;
  _previous_opponent_health = _opponent->state().health;
  _hit_landed = false;
  _got_hit = false;
  ai_agent->decay_exploration();

  Character::update(delta_time);
}

void AICharacter::render(SDL_Renderer *renderer) const {

  Character::render(renderer);

  draw_radar(renderer);
}

bool AICharacter::is_opponent_in_radar() const {
  if (!_opponent)
    return false;
  Vector2f diff = _opponent->mover().position().sub(mover().position());
  float dist = diff.mag();
  return dist <= _radar_radius;
}

void AICharacter::draw_radar(SDL_Renderer *renderer) const {

  Vector2f center = mover().position();
  int segments = 60;
  float angle_step =
      2.0f * static_cast<float>(M_PI) / static_cast<float>(segments);

  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128);

  for (int i = 0; i < segments; i++) {
    float angle1 = i * angle_step;
    float angle2 = (i + 1) * angle_step;

    float x1 = center.x + _radar_radius * std::cos(angle1);
    float y1 = center.y + _radar_radius * std::sin(angle1);
    float x2 = center.x + _radar_radius * std::cos(angle2);
    float y2 = center.y + _radar_radius * std::sin(angle2);

    SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1),
                       static_cast<int>(x2), static_cast<int>(y2));
  }

  if (is_opponent_in_radar() && _opponent) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    int x1 = (int)mover().position().x;
    int y1 = (int)mover().position().y;
    int x2 = (int)_opponent->mover().position().x;
    int y2 = (int)_opponent->mover().position().y;
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
  }
}

void AICharacter::execute_action(ai::Action action) {
  _time_since_last_action = 0.0f;

  std::string action_name;
  // TODO: refactor this into a function
  switch (action) {
  case ai::Action::MOVE_LEFT:
    action_name = "MOVE_LEFT";
    break;
  case ai::Action::MOVE_RIGHT:
    action_name = "MOVE_RIGHT";
    break;
  case ai::Action::MOVE_UP:
    action_name = "MOVE_UP";
    break;
  case ai::Action::MOVE_DOWN:
    action_name = "MOVE_DOWN";
    break;
  case ai::Action::LIGHT_PUNCH:
    action_name = "LIGHT_PUNCH";
    break;
  case ai::Action::HEAVY_PUNCH:
    action_name = "HEAVY_PUNCH";
    break;
  case ai::Action::LIGHT_KICK:
    action_name = "LIGHT_KICK";
    break;
  case ai::Action::HEAVY_KICK:
    action_name = "HEAVY_KICK";
    break;
  case ai::Action::BLOCK:
    action_name = "BLOCK";
    break;
  case ai::Action::IDLE:
    action_name = "IDLE";
    break;
  }

  std::cout << "[AI Action] " << action_name << std::endl;

  // TODO: this amount should be decided by the agent
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

void AICharacter::start_new_episode() {
  _training_episode++;
  log_episode_start();
  reset();
}
void AICharacter::set_opponent(Character *opponent) {
  _opponent = opponent;
  _previous_opponent_health = opponent->state().health;
}
void AICharacter::handle_defeat() {
  Character::handle_defeat();
  log_episode_end();
}
void AICharacter::log_combat_event(const std::string &event_type,
                                   const std::string &details) {
  std::cout << "【" << event_type << "】 " << details << std::endl;
}
void AICharacter::log_episode_end() {
  std::cout << "\n╔════════════════════════════════════════╗\n";
  std::cout << "║ Episode " << _training_episode << " Complete!\n";
  std::cout << "║ Final Health: " << state().health << "/" << state().max_health
            << std::endl;
  std::cout << "║ Hits Landed: " << _hits_landed << std::endl;
  std::cout << "║ Hits Taken: " << _hits_taken << std::endl;
  std::cout << "║ Average Distance: " << _average_distance << std::endl;
  std::cout << "╚════════════════════════════════════════╝\n\n";
}
void AICharacter::log_episode_start() {
  std::cout << "\n╔════════════════════════════════════════╗\n";
  std::cout << "║ Starting Episode " << _training_episode << std::endl;
  std::cout << "║ AI Health: " << state().health << "/" << state().max_health
            << std::endl;
  std::cout << "║ Exploration Rate: " << ai_agent->get_exploration_rate()
            << std::endl;
  std::cout << "╚════════════════════════════════════════╝\n";
}
void AICharacter::on_hit_landed() {
  _hit_landed = true;
  _hits_landed++;
}

void AICharacter::on_got_hit() {
  _got_hit = true;
  _hits_taken++;
}
} // namespace entities
} // namespace wbz
