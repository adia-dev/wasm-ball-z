#include "character.hpp"
#include "math/vector2.hpp"
#include "text/text_renderer.hpp"
#include "utils/r.hpp"
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <iostream>

namespace wbz {
namespace entities {

Character::Character(const Sprite &sprite, const CombatStats &stats)
    : _sprite(sprite), _stats(stats), _current_combat_state(CombatState::IDLE),
      _staring_at(nullptr), _is_looking_right(true), _state_timer(0.0f),
      _invulnerability_timer(0.0f), _recovery_timer(0.0f), _combo_counter(0) {

  _state.health = _state.max_health = stats.max_health;
  _state.stamina = _state.max_stamina = stats.max_stamina;
  _state.defense = stats.base_defense;
  _state.is_invulnerable = false;
  _state.hit_stun_timer = 0.0f;
  _state.attack_timer = 0.0f;

  _hurt_box = HitBox(Vector2f(50, 100), Vector2f(0, 0));
  _current_hit_box = HitBox();
  _current_hit_box.is_active = false;

  register_basic_attacks();

  _rect.w = 64;
  _rect.h = 64;
}

void Character::register_basic_attacks() {

  _attacks["light_punch"] =
      Attack("light_punch", 8, 40.0f, 3.0f, 2.0f, 6.0f, 200.0f, 5, true,
             "punch_light", Vector2f(40, 20), Vector2f(30, 0));

  _attacks["light_kick"] =
      Attack("light_kick", 10, 45.0f, 4.0f, 2.0f, 7.0f, 250.0f, 8, true,
             "kick_light", Vector2f(45, 25), Vector2f(35, 10));

  _attacks["heavy_punch"] =
      Attack("heavy_punch", 20, 50.0f, 8.0f, 3.0f, 12.0f, 400.0f, 15, false,
             "punch_heavy", Vector2f(50, 30), Vector2f(40, 0));

  _attacks["heavy_kick"] =
      Attack("heavy_kick", 25, 60.0f, 10.0f, 4.0f, 15.0f, 500.0f, 20, false,
             "kick_heavy", Vector2f(60, 35), Vector2f(45, 10));
}

void Character::update(double delta_time) {
  update_timers(delta_time);
  update_combat_state(delta_time);
  update_combos(delta_time);
  apply_movement_forces();

  Vector2f friction_force =
      _mover.velocity().mul(-1.0f).normalized().mul(800.f);
  _mover.add_force(friction_force);
  _mover.update(delta_time);

  const float BOUNCE_FACTOR = 0.5f;
  const float BORDER_MARGIN = 50.0f;

  Vector2f pos = _mover.position();
  Vector2f vel = _mover.velocity();

  if (pos.x < BORDER_MARGIN) {
    pos.x = BORDER_MARGIN;
    if (vel.x < 0) {
      vel = Vector2f(-vel.x * BOUNCE_FACTOR, vel.y);
      _mover.set_velocity(vel);
    }
  } else if (pos.x > 800 - BORDER_MARGIN) {
    pos.x = 800 - BORDER_MARGIN;
    if (vel.x > 0) {
      vel = Vector2f(-vel.x * BOUNCE_FACTOR, vel.y);
      _mover.set_velocity(vel);
    }
  }

  if (pos.y < BORDER_MARGIN) {
    pos.y = BORDER_MARGIN;
    if (vel.y < 0) {
      vel = Vector2f(vel.x, -vel.y * BOUNCE_FACTOR);
      _mover.set_velocity(vel);
    }
  } else if (pos.y > 600 - BORDER_MARGIN) {
    pos.y = 600 - BORDER_MARGIN;
    if (vel.y > 0) {
      vel = Vector2f(vel.x, -vel.y * BOUNCE_FACTOR);
      _mover.set_velocity(vel);
    }
  }

  _mover.set_position(pos);

  _sprite.set_frame(_animator.frame());
  SDL_Rect current_frame = _animator.frame();
  float adjusted_x = pos.x - current_frame.w / 2.0f;
  float adjusted_y = pos.y - current_frame.h / 2.0f;
  _sprite.set_position(static_cast<int>(adjusted_x),
                       static_cast<int>(adjusted_y));

  _animator.update(delta_time);

  if (_staring_at != nullptr) {
    _is_looking_right = (_staring_at->x - pos.x) < 0;
  }
}

void Character::update_timers(double delta_time) {

  if (_state.hit_stun_timer > 0.0f) {
    _state.hit_stun_timer -= delta_time;
  }

  if (_state.attack_timer > 0.0f) {
    _state.attack_timer -= delta_time;
  }

  if (_recovery_timer > 0.0f) {
    _recovery_timer -= delta_time;
  }

  if (_invulnerability_timer > 0.0f) {
    _invulnerability_timer -= delta_time;
    _state.is_invulnerable = _invulnerability_timer > 0.0f;
  }
}

void Character::update_combat_state(double delta_time) {
  switch (_current_combat_state) {
  case CombatState::STUNNED:
    if (_state.hit_stun_timer <= 0.0f) {
      set_combat_state(CombatState::RECOVERY);
      _recovery_timer = 0.5f;
    }
    break;

  case CombatState::ATTACKING:
    if (_state.attack_timer <= 0.0f) {
      _current_hit_box.is_active = false;
      set_combat_state(CombatState::IDLE);
    }
    break;

  case CombatState::RECOVERY:
    if (_recovery_timer <= 0.0f) {
      set_combat_state(CombatState::IDLE);
    }
    break;

  case CombatState::IDLE:

    _state.restore_stamina(1);
    break;

  default:
    break;
  }
}

void Character::update_combos(double delta_time) {
  const float COMBO_WINDOW = 1.0f;

  while (!_recent_hit_times.empty() &&
         _recent_hit_times.front() >= COMBO_WINDOW) {
    _recent_hit_times.pop();
  }

  if (_recent_hit_times.empty()) {
    _combo_counter = 0;
  }
}

bool Character::perform_attack(const std::string &attack_name) {
  if (!_attacks.count(attack_name)) {
    return false;
  }

  const Attack &attack = _attacks[attack_name];

  if (!_state.can_perform_action(attack.stamina_cost)) {
    return false;
  }

  _state.stamina -= attack.stamina_cost;

  set_combat_state(CombatState::ATTACKING);
  _state.attack_timer =
      attack.startup_frames + attack.active_frames + attack.recovery_frames;

  _current_hit_box.size = attack.hit_box_size;
  _current_hit_box.offset = attack.hit_box_offset;
  _current_hit_box.offset.x *= _is_looking_right ? 1 : -1;
  _current_hit_box.is_active = false;

  _animator.play(attack.animation_name);
  return true;
}

bool Character::is_hit_connecting(const Character &other,
                                  const Attack &attack) const {

  if (_current_combat_state != CombatState::ATTACKING ||
      !_current_hit_box.is_active) {
    return false;
  }

  if (!other.is_vulnerable()) {
    return false;
  }

  return check_hit_box_collision(_current_hit_box, other);
}

bool Character::check_hit_box_collision(const HitBox &attack_box,
                                        const Character &defender) const {

  Vector2f attacker_pos = _mover.position();
  Vector2f defender_pos = defender.mover().position();

  if (std::isnan(attacker_pos.x) || std::isnan(attacker_pos.y) ||
      std::isnan(defender_pos.x) || std::isnan(defender_pos.y)) {
    return false;
  }

  Vector2f attack_min = attacker_pos.add(attack_box.offset);
  Vector2f attack_max = attack_min.add(attack_box.size);
  Vector2f hurt_min = defender_pos.add(defender._hurt_box.offset);
  Vector2f hurt_max = hurt_min.add(defender._hurt_box.size);

  const float EPSILON = 0.001f;
  return !(attack_max.x + EPSILON < hurt_min.x ||
           attack_min.x > hurt_max.x + EPSILON ||
           attack_max.y + EPSILON < hurt_min.y ||
           attack_min.y > hurt_max.y + EPSILON);
}

void Character::apply_knockback(const Vector2f &direction, float force) {

  if (force < 0.0f || std::isnan(force)) {
    return;
  }

  if (std::isnan(direction.x) || std::isnan(direction.y)) {
    return;
  }

  Vector2f safe_direction = direction;
  if (direction.mag() < 0.0001f) {
    safe_direction = Vector2f(1.0f, 0.0f);
  } else {
    safe_direction = direction.normalized();
  }

  float scaled_force = force / std::max(0.1f, _stats.weight);
  _mover.add_force(safe_direction.mul(scaled_force));
}

void Character::set_combat_state(CombatState new_state) {

  switch (_current_combat_state) {
  case CombatState::ATTACKING:
    _current_hit_box.is_active = false;
    break;
  case CombatState::BLOCKING:

    break;
  default:
    break;
  }

  _current_combat_state = new_state;

  switch (new_state) {
  case CombatState::IDLE:
    _animator.play("Idle");
    break;
  case CombatState::BLOCKING:
    _animator.play("Block");
    break;
  case CombatState::STUNNED:
    _animator.play("Stunned");
    break;
  case CombatState::RECOVERY:
    _animator.play("Recovery");
    break;
  default:
    break;
  }
}

bool Character::can_attack() const {
  return _current_combat_state == CombatState::IDLE ||
         _current_combat_state == CombatState::WALKING ||
         (_current_combat_state == CombatState::ATTACKING &&
          _attacks.at("idle").can_be_canceled);
}

bool Character::can_block() const {
  return _current_combat_state == CombatState::IDLE ||
         _current_combat_state == CombatState::WALKING;
}

bool Character::is_stunned() const {
  return _current_combat_state == CombatState::STUNNED;
}

bool Character::is_in_recovery() const {
  return _current_combat_state == CombatState::RECOVERY;
}

bool Character::is_vulnerable() const {
  return _invulnerability_timer <= 0.0f &&
         _current_combat_state != CombatState::BLOCKING;
}

float Character::get_attack_multiplier() const {
  float combo_multiplier = 1.0f - (_combo_counter * 0.1f);
  return std::max(0.5f, combo_multiplier) * _stats.attack_speed_modifier;
}

float Character::get_defense_multiplier() const {
  return _current_combat_state == CombatState::BLOCKING ? 0.5f : 1.0f;
}

void Character::apply_movement_forces() {

  if (_current_combat_state == CombatState::STUNNED ||
      _current_combat_state == CombatState::ATTACKING) {
    return;
  }

  float speed_multiplier = 1.0f;
  if (_current_combat_state == CombatState::BLOCKING) {
    speed_multiplier = 0.5f;
  } else if (_current_combat_state == CombatState::CROUCHING) {
    speed_multiplier = 0.3f;
  }

  float final_speed = _stats.movement_speed * speed_multiplier;
}

void Character::render(SDL_Renderer *renderer) const {

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  _sprite.render(renderer, !_is_looking_right);

  render_debug_boxes(renderer);

  render_health_bar(renderer);
  render_stamina_bar(renderer);
  render_state_info(renderer);

  render_floating_text(renderer);
}

void Character::render_debug_boxes(SDL_Renderer *renderer) const {

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
  SDL_Rect bounds = {static_cast<int>(_mover.position().x - _rect.w / 2),
                     static_cast<int>(_mover.position().y - _rect.h / 2),
                     _rect.w, _rect.h};
  SDL_RenderDrawRect(renderer, &bounds);

  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);
  Vector2f hurt_pos = _mover.position().add(_hurt_box.offset);
  SDL_Rect hurt_rect = {
      static_cast<int>(hurt_pos.x), static_cast<int>(hurt_pos.y),
      static_cast<int>(_hurt_box.size.x), static_cast<int>(_hurt_box.size.y)};
  SDL_RenderDrawRect(renderer, &hurt_rect);

  if (_current_combat_state == CombatState::ATTACKING &&
      _current_hit_box.is_active) {

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 64);
    Vector2f hit_pos = _mover.position().add(_current_hit_box.offset);
    SDL_Rect hit_rect = {static_cast<int>(hit_pos.x),
                         static_cast<int>(hit_pos.y),
                         static_cast<int>(_current_hit_box.size.x),
                         static_cast<int>(_current_hit_box.size.y)};
    SDL_RenderFillRect(renderer, &hit_rect);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &hit_rect);

    SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);
    int center_x = hit_rect.x + hit_rect.w / 2;
    int center_y = hit_rect.y + hit_rect.h / 2;
    SDL_RenderDrawLine(renderer, center_x, center_y,
                       center_x + (_is_looking_right ? 20 : -20), center_y);
  }
}

void Character::render_state_info(SDL_Renderer *renderer) const {
  const int TEXT_HEIGHT = 15;
  Vector2f pos = _mover.position();

  std::string state_text = get_state_text();
  render_text(renderer, state_text, pos.x - 30, pos.y - _rect.h - 60,
              {255, 255, 255, 255});

  std::string info_text = std::string("HP: ") + std::to_string(_state.health) +
                          "/" + std::to_string(_state.max_health);
  render_text(renderer, info_text, pos.x - 30, pos.y - _rect.h - 80,
              {255, 255, 255, 255});

  Vector2f vel = _mover.velocity();
  std::string vel_text = "vel: (" + std::to_string(static_cast<int>(vel.x)) +
                         "," + std::to_string(static_cast<int>(vel.y)) + ")";
  render_text(renderer, vel_text, pos.x - 30, pos.y - _rect.h - 100,
              {200, 200, 200, 255});
}

void Character::render_text(SDL_Renderer *renderer, const std::string &text,
                            float x, float y, SDL_Color color) const {
  TextRenderer::instance().render_text(renderer, text, static_cast<int>(x),
                                       static_cast<int>(y), color, 16);
}

std::string Character::get_state_text() const {
  switch (_current_combat_state) {
  case CombatState::IDLE:
    return "IDLE";
  case CombatState::WALKING:
    return "WALKING";
  case CombatState::ATTACKING:
    return "ATTACK!";
  case CombatState::BLOCKING:
    return "BLOCK";
  case CombatState::STUNNED:
    return "STUNNED";
  case CombatState::JUMPING:
    return "JUMP";
  case CombatState::CROUCHING:
    return "CROUCH";
  case CombatState::DASHING:
    return "DASH";
  case CombatState::RECOVERY:
    return "RECOVERY";
  default:
    return "UNKNOWN";
  }
}

void Character::render_health_bar(SDL_Renderer *renderer) const {
  const int BAR_WIDTH = 50;
  const int BAR_HEIGHT = 5;
  const int BAR_Y_OFFSET = 40;

  SDL_Rect health_bar = {static_cast<int>(_mover.position().x - BAR_WIDTH / 2),
                         static_cast<int>(_mover.position().y - BAR_Y_OFFSET),
                         BAR_WIDTH, BAR_HEIGHT};

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &health_bar);

  float health_ratio = static_cast<float>(_state.health) / _state.max_health;
  health_bar.w = static_cast<int>(BAR_WIDTH * health_ratio);
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  SDL_RenderFillRect(renderer, &health_bar);
}

void Character::render_stamina_bar(SDL_Renderer *renderer) const {
  const int BAR_WIDTH = 40;
  const int BAR_HEIGHT = 3;
  const int BAR_Y_OFFSET = 35;

  SDL_Rect stamina_bar = {static_cast<int>(_mover.position().x - BAR_WIDTH / 2),
                          static_cast<int>(_mover.position().y - BAR_Y_OFFSET),
                          BAR_WIDTH, BAR_HEIGHT};

  SDL_SetRenderDrawColor(renderer, 64, 64, 255, 255);
  SDL_RenderFillRect(renderer, &stamina_bar);

  float stamina_ratio = static_cast<float>(_state.stamina) / _state.max_stamina;
  stamina_bar.w = static_cast<int>(BAR_WIDTH * stamina_ratio);
  SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
  SDL_RenderFillRect(renderer, &stamina_bar);
}

void Character::stare_at(const Vector2f *target) { _staring_at = target; }

void Character::add_floating_text(const std::string &text,
                                  const Vector2f &position,
                                  const SDL_Color &color) {

  float random_angle = (std::rand() % 60 - 30) * 3.14f / 180.0f;
  float speed = 200.0f;

  Vector2f velocity(std::cos(random_angle) * speed,
                    std::sin(random_angle) * speed - 300.0f);

  int text_size = 16;
  if (text.find("CRITICAL") != std::string::npos) {
    text_size = 24;
  } else if (text.find("BLOCK") != std::string::npos) {
    text_size = 20;
  }

  FloatingText ft(text, position, velocity, 1.5f, color);
  ft.size = text_size;
  _floating_texts.emplace_back(std::move(ft));

  std::cout << "Combat Event: " << text << " at position (" << position.x
            << ", " << position.y << ")\n";
}

void Character::update_floating_texts(double delta_time) {
  for (auto &text : _floating_texts) {
    text.position = text.position.add(text.velocity.mul(delta_time));
    text.lifetime -= delta_time;

    text.velocity = text.velocity.mul(0.95f);
  }

  _floating_texts.erase(std::remove_if(_floating_texts.begin(),
                                       _floating_texts.end(),
                                       [](const FloatingText &text) {
                                         return text.lifetime <= 0;
                                       }),
                        _floating_texts.end());
}

void Character::render_floating_text(SDL_Renderer *renderer) const {
  for (const auto &text : _floating_texts) {

    SDL_Color color = text.color;
    float fade = std::min(1.0f, text.lifetime);
    fade = fade * fade;
    color.a = static_cast<Uint8>(255 * fade);

    float scaled_x = text.position.x;
    float scaled_y = text.position.y;

    SDL_Color shadow_color = {0, 0, 0, color.a};
    TextRenderer::instance().render_text(
        renderer, text.text, static_cast<int>(scaled_x + 2),
        static_cast<int>(scaled_y + 2), shadow_color,
        static_cast<int>(text.size));

    TextRenderer::instance().render_text(
        renderer, text.text, static_cast<int>(scaled_x),
        static_cast<int>(scaled_y), color, static_cast<int>(text.size));
  }
}

void Character::apply_hit(const Attack &attack, const Vector2f &attacker_pos) {
  if (_invulnerability_timer > 0.0f) {
    add_floating_text("BLOCK!", _mover.position(), {255, 255, 0, 255});
    return;
  }

  apply_damage(attack.damage);

  // Keep the knockback and animation code
  Vector2f knockback_dir = (_mover.position().sub(attacker_pos)).normalized();
  apply_knockback(knockback_dir, attack.knockback_force);
  _animator.play("Hit");
}

void Character::reset() {
  _state.health = _state.max_health;
  _state.stamina = _state.max_stamina;
  _state.is_invulnerable = false;
  _state.hit_stun_timer = 0.0f;
  _state.attack_timer = 0.0f;
  set_combat_state(CombatState::IDLE);
  _mover.set_velocity(Vector2f::zero());
}
void Character::apply_damage(int raw_damage) {
  if (_state.is_invulnerable)
    return;

  float defense_multiplier = get_defense_multiplier();
  int final_damage = static_cast<int>(raw_damage * defense_multiplier);

  _state.health = std::max(0, _state.health - final_damage);

  // Add visual and audio feedback
  Vector2f damage_pos = _mover.position().add(Vector2f(0, -30));
  SDL_Color color = {255, 0, 0, 255};

  if (defense_multiplier < 1.0f) {
    add_floating_text("BLOCKED! " + std::to_string(final_damage), damage_pos,
                      {255, 255, 0, 255});
  } else if (final_damage >= 20) {
    add_floating_text("CRITICAL! " + std::to_string(final_damage), damage_pos,
                      {255, 165, 0, 255});
  } else {
    add_floating_text(std::to_string(final_damage), damage_pos, color);
  }

  // Trigger hit reactions
  if (_state.health <= 0) {
    handle_defeat();
  } else {
    set_combat_state(CombatState::STUNNED);
    _state.hit_stun_timer = 0.3f;
  }
}
void Character::handle_defeat() {
  set_combat_state(CombatState::STUNNED);
  add_floating_text("DEFEATED!", _mover.position(), {255, 0, 0, 255});
}
} // namespace entities
} // namespace wbz
