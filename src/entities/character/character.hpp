#pragma once

#include <mover/mover.hpp>
#include <string>

#include "SDL_render.h"

namespace wbz {
namespace entities {
class Character {
public:
  Character() : _mover(1000.0f), _name("<unnamed>") {}
  explicit Character(const std::string &name) : _mover(10.0f), _name(name) {}

  const std::string &name() const { return _name; }
  Mover &mover() { return _mover; }

  void set_name(const std::string &name) { _name = name; }

  void update(double delta_time) {
    Vector2f friction_force =
        _mover.velocity().mul(-1.0f).normalized().mul(800.f);
    _mover.add_force(friction_force);

    _mover.update(delta_time);
  }

  void render(SDL_Renderer *renderer) const {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_Rect character_rect{
        .x = static_cast<int>(_mover.position().x),
        .y = static_cast<int>(_mover.position().y),
        .w = 10,
        .h = 20,
    };

    SDL_RenderFillRect(renderer, &character_rect);
  }

private:
  Mover _mover;
  std::string _name;
};
} // namespace entities
} // namespace wbz
