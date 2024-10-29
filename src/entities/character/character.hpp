#pragma once

#include <entities/entity.hpp>
#include <mover/mover.hpp>
#include <sprite/sprite.hpp>

#include "SDL_render.h"

namespace wbz {
namespace entities {
class Character : public Entity {
public:
  explicit Character(const Sprite &sprite) : _sprite(sprite) {}

  Mover &mover();

  void update(double delta_time) override;
  void render(SDL_Renderer *renderer) const override;

private:
  Mover _mover;
  Sprite _sprite;
};
} // namespace entities
} // namespace wbz
