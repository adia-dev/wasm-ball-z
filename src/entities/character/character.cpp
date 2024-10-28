#include "character.hpp"

namespace wbz {
namespace entities {

Character::Character() : _name("<unnamed>") {}

Character::Character(const std::string &name) : _name(name) {}

const std::string &Character::name() const { return _name; }

const SDL_Rect &Character::rect() const { return _rect; }

Mover &Character::mover() { return _mover; }

void Character::set_name(const std::string &name) { _name = name; }

void Character::set_width(int width) { _rect.w = width; }

void Character::set_height(int height) { _rect.h = height; }

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
}

void Character::render(SDL_Renderer *renderer) const {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_Rect character_rect{
      .x = static_cast<int>(_mover.position().x),
      .y = static_cast<int>(_mover.position().y),
      .w = _rect.w,
      .h = _rect.h,
  };

  SDL_RenderFillRect(renderer, &character_rect);
}
} // namespace entities
} // namespace wbz
