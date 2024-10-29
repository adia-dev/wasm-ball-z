#include "sprite.hpp"
#include "managers/resource_manager/resource_manager.hpp"

#include <SDL_render.h>
#include <iostream>

namespace wbz {
Sprite::Sprite(const std::string &texture_id, const SDL_Rect &src_rect,
               const SDL_Rect &dst_rect)
    : _texture_id(texture_id), _src_rect(src_rect), _dst_rect(dst_rect) {}

void Sprite::render(SDL_Renderer *renderer) const {
  auto texture = managers::ResourceManager::get_texture(renderer, _texture_id);
  if (texture == nullptr) {
    std::cerr << "Cannot render sprite: texture is null\n";
    return;
  }

  SDL_RenderCopy(renderer, texture.get(), &_src_rect, &_dst_rect);
}

} // namespace wbz
