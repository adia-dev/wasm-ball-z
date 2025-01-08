#pragma once
#include "SDL_render.h"
#include "math/vector2.hpp"
#include "sprite/animator/animator.hpp"
#include <entities/entity.hpp>
#include <mover/mover.hpp>
#include <queue>
#include <sprite/sprite.hpp>
#include <unordered_map>

namespace wbz {
namespace entities {

enum class CombatState {
  IDLE,
  WALKING,
  ATTACKING,
  BLOCKING,
  STUNNED,
  JUMPING,
  CROUCHING,
  DASHING,
  RECOVERY
};

struct CombatStats {
  int max_health;
  int max_stamina;
  float movement_speed;
  float jump_force;
  float weight;
  int base_defense;
  float attack_speed_modifier;

  CombatStats(int health = 100, int stamina = 100, float speed = 500.0f,
              float jump = 800.0f, float weight = 1.0f, int defense = 10,
              float atk_speed = 1.0f)
      : max_health(health), max_stamina(stamina), movement_speed(speed),
        jump_force(jump), weight(weight), base_defense(defense),
        attack_speed_modifier(atk_speed) {}
};

struct Attack {
  std::string name;
  int damage;
  float range;
  float startup_frames;
  float active_frames;
  float recovery_frames;
  float knockback_force;
  int stamina_cost;
  bool can_be_canceled;
  std::string animation_name;
  Vector2f hit_box_size;
  Vector2f hit_box_offset;

  Attack(const std::string &n = "", int dmg = 10, float rng = 50.0f,
         float startup = 5.0f, float active = 3.0f, float recovery = 10.0f,
         float knockback = 500.0f, int stamina = 10, bool cancelable = false,
         const std::string &anim = "",
         const Vector2f &box_size = Vector2f(50, 50),
         const Vector2f &box_offset = Vector2f(50, 0))
      : name(n), damage(dmg), range(rng), startup_frames(startup),
        active_frames(active), recovery_frames(recovery),
        knockback_force(knockback), stamina_cost(stamina),
        can_be_canceled(cancelable), animation_name(anim),
        hit_box_size(box_size), hit_box_offset(box_offset) {}
};

struct HitBox {
  Vector2f size;
  Vector2f offset;
  bool is_active;

  HitBox(const Vector2f &s = Vector2f(50, 100),
         const Vector2f &o = Vector2f(0, 0))
      : size(s), offset(o), is_active(true) {}
};

struct CharacterState {
  int health;
  int max_health;
  int stamina;
  int max_stamina;
  int defense;
  bool is_invulnerable;
  float hit_stun_timer = 0.0f;
  float attack_timer = 0.0f;

  bool is_alive() const { return health > 0; }

  void apply_damage(int damage) {
    if (!is_invulnerable) {
      health = std::max(0, health - std::max(0, damage - defense / 2));
    }
  }

  void heal(int amount) { health = std::min(max_health, health + amount); }

  void restore_stamina(int amount) {
    stamina = std::min(max_stamina, stamina + amount);
  }

  bool can_perform_action(int stamina_cost) const {
    return stamina >= stamina_cost && hit_stun_timer <= 0.0f;
  }
};

struct FloatingText {
  std::string text;
  Vector2f position;
  Vector2f velocity;
  float lifetime;
  SDL_Color color;
  int size;

  FloatingText(const std::string &t, const Vector2f &pos, const Vector2f &vel,
               float life, const SDL_Color &col, int size = 16)
      : text(t), position(pos), velocity(vel), lifetime(life), color(col),
        size(size) {}
};

class Character : public Entity {
public:
  explicit Character(const Sprite &sprite,
                     const CombatStats &stats = CombatStats());

  Mover &mover() { return _mover; }
  const Mover &mover() const { return _mover; }

  CharacterState &state() { return _state; }
  const CharacterState &state() const { return _state; }

  Animator &animator() { return _animator; }

  void reset();
  void apply_damage(int raw_damage);

  void set_combat_state(CombatState new_state);
  CombatState get_combat_state() const { return _current_combat_state; }

  bool perform_attack(const std::string &attack_name);
  bool is_hit_connecting(const Character &other, const Attack &attack) const;
  void apply_hit(const Attack &attack, const Vector2f &attacker_pos);

  void apply_knockback(const Vector2f &direction, float force);
  bool is_colliding_with(const Character &other) const;

  bool can_attack() const;
  bool can_block() const;
  bool is_stunned() const;
  bool is_in_recovery() const;
  bool is_vulnerable() const;

  float get_attack_multiplier() const;
  float get_defense_multiplier() const;
  int get_combo_count() const { return _combo_counter; }

  void update(double delta_time) override;
  void render(SDL_Renderer *renderer) const override;

  void stare_at(const Vector2f *target);
  bool is_facing_right() const { return _is_looking_right; }

  bool check_hit_box_collision(const HitBox &attack_box,
                               const Character &defender) const;

protected:
  virtual void handle_defeat();

private:
  Mover _mover;
  Sprite _sprite;
  Animator _animator;

  CombatStats _stats;
  int _current_health;
  int _current_stamina;
  CombatState _current_combat_state;
  CharacterState _state;

  float _state_timer;
  float _invulnerability_timer;
  float _recovery_timer;
  int _combo_counter;
  std::queue<float> _recent_hit_times;

  const Vector2f *_staring_at;
  bool _is_looking_right;

  std::unordered_map<std::string, Attack> _attacks;
  HitBox _hurt_box;
  HitBox _current_hit_box;

  mutable std::vector<FloatingText> _floating_texts;

  void update_combat_state(double delta_time);
  void update_timers(double delta_time);
  void update_combos(double delta_time);
  void update_floating_texts(double delta_time);
  void apply_movement_forces();
  void register_basic_attacks();
  void render_debug_boxes(SDL_Renderer *renderer) const;
  void render_health_bar(SDL_Renderer *renderer) const;
  void render_stamina_bar(SDL_Renderer *renderer) const;
  void render_state_info(SDL_Renderer *renderer) const;
  void render_floating_text(SDL_Renderer *renderer) const;
  void render_text(SDL_Renderer *renderer, const std::string &text, float x,
                   float y, SDL_Color color) const;
  std::string get_state_text() const;
  void add_floating_text(const std::string &text, const Vector2f &position,
                         const SDL_Color &color);
};

} // namespace entities
} // namespace wbz
