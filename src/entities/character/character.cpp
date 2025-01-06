#include "character.hpp"
#include "math/vector2.hpp"

namespace wbz {
namespace entities {

Mover &Character::mover() { return _mover; }

void Character::update(double delta_time) {
  Vector2f friction_force =
      _mover.velocity().mul(-1.0f).normalized().mul(800.f);
  _mover.add_force(friction_force);

  _mover.update(delta_time);

  if (_mover.position().x + _rect.w < 0) {
    _mover.set_position({static_cast<float>(800 - 1), _mover.position().y});
  } else if (_mover.position().x > 800) {
    _mover.set_position({static_cast<float>(_rect.w + 1), _mover.position().y});
  }

  // Update frame and position
  _sprite.set_frame(_animator.frame());
  SDL_Rect current_frame = _animator.frame();
  float adjusted_x = _mover.position().x - current_frame.w / 2.0f;
  float adjusted_y = _mover.position().y - current_frame.h / 2.0f;
  _sprite.set_position(static_cast<int>(adjusted_x),
                       static_cast<int>(adjusted_y));

  _animator.update(delta_time);

  if (_staring_at != nullptr) {
    _is_looking_right = (_staring_at->x - _mover.position().x) < 0;
  }
}

void Character::render(SDL_Renderer *renderer) const {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  _sprite.render(renderer, !_is_looking_right);
}

Animator &Character::animator() { return _animator; }

void Character::stare_at(const Vector2f *target) { _staring_at = target; }
} // namespace entities
} // namespace wbz
