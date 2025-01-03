#pragma once

#include "SDL_render.h"
#include <SDL_rect.h>
#include <string>

namespace wbz {

class Sprite {
public:
  Sprite(const std::string &texture_id, const SDL_Rect &src_rect,
         const SDL_Rect &dst_rect);
  void render(SDL_Renderer *renderer, bool flip = false) const;

  void set_frame(const SDL_Rect &frame);
  void set_position(int x, int y);

private:
  std::string _texture_id;
  SDL_Rect _src_rect, _dst_rect;
};

} // namespace wbz
