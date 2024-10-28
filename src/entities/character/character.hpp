#pragma once

#include <mover/mover.hpp>
#include <string>

#include "SDL_render.h"

namespace wbz {
namespace entities {
class Character {
public:
  Character();
  explicit Character(const std::string &name);

  const std::string &name() const;
  Mover &mover();

  void set_width(int width);
  void set_height(int height);
  void set_name(const std::string &name);

  void update(double delta_time);
  void render(SDL_Renderer *renderer) const;

  const SDL_Rect &rect() const;

private:
  Mover _mover;
  SDL_Rect _rect;
  std::string _name;
};
} // namespace entities
} // namespace wbz
