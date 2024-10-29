#include "character.hpp"

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

  _sprite.set_position(_mover.position().x, _mover.position().y);
}

void Character::render(SDL_Renderer *renderer) const {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  _sprite.render(renderer);
}
} // namespace entities
} // namespace wbz
