#include "map.hpp"
#include "managers/resource_manager/resource_manager.hpp"

namespace wbz {

Map::Map(const std::string &map_path) : _texture_id(map_path) {}

Map::Map(const std::string &map_path, const SDL_Rect &map_rect)
    : _texture_id(map_path), _map_rect(map_rect) {}

void Map::set_map_file(const std::string &map_path) { _texture_id = map_path; }

void Map::set_map_rect(const SDL_Rect &map_rect) { _map_rect = map_rect; }

void Map::render(SDL_Renderer *renderer) {
  auto texture = managers::ResourceManager::get_texture(renderer, _texture_id);
  if (texture == nullptr) {
    std::cerr << "Cannot render map: texture is null\n";
    return;
  }

  SDL_RenderCopy(renderer, texture.get(), &_map_rect, nullptr);
}

void Map::update(float delta_time) {}
} // namespace wbz
