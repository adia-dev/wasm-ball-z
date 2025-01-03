#pragma once

#include <entities/entity.hpp>
#include <mover/mover.hpp>
#include <sprite/sprite.hpp>

#include "SDL_render.h"
#include "math/vector2.hpp"
#include "sprite/animator/animator.hpp"

namespace wbz {
namespace entities {
class Character : public Entity {
public:
  explicit Character(const Sprite &sprite) : _sprite(sprite) {}

  Mover &mover();
  Animator &animator();

  void update(double delta_time) override;
  void render(SDL_Renderer *renderer) const override;
  void stare_at(const Vector2f *target);

private:
  Mover _mover;
  Sprite _sprite;
  Animator _animator;

  const Vector2f *_staring_at = nullptr;
  bool _is_looking_right = true;
};
} // namespace entities
} // namespace wbz
