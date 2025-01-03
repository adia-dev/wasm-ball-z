#include "sprite.hpp"
#include "SDL_rect.h"
#include "managers/resource_manager/resource_manager.hpp"

#include <SDL_render.h>
#include <iostream>

namespace wbz {
Sprite::Sprite(const std::string &texture_id, const SDL_Rect &src_rect,
               const SDL_Rect &dst_rect)
    : _texture_id(texture_id), _src_rect(src_rect), _dst_rect(dst_rect) {}

void Sprite::render(SDL_Renderer *renderer, bool flip) const {
  auto texture = managers::ResourceManager::get_texture(renderer, _texture_id);
  if (texture == nullptr) {
    std::cerr << "Cannot render sprite: texture is null\n";
    return;
  }

  SDL_Point origin{.x = _src_rect.w / 2, .y = _src_rect.h};
  SDL_RenderCopyEx(renderer, texture.get(), &_src_rect, &_dst_rect, 0.0,
                   &origin, flip ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
}

void Sprite::set_frame(const SDL_Rect &frame) {
  _src_rect = frame;
  _dst_rect.w = frame.w;
  _dst_rect.h = frame.h;
}

void Sprite::set_position(int x, int y) {
  _dst_rect.x = x;
  _dst_rect.y = y;
}
} // namespace wbz
