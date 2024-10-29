#pragma once

#include <SDL_render.h>

namespace wbz {
namespace entities {

class Entity {
public:
  void set_width(int width) { _rect.w = width; }
  void set_height(int height) { _rect.h = height; }

  virtual void update(double delta_time) = 0;
  virtual void render(SDL_Renderer *renderer) const = 0;

  const SDL_Rect &rect() const { return _rect; }

protected:
  SDL_Rect _rect;
};
} // namespace entities
} // namespace wbz
