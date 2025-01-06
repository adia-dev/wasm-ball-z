#pragma once

#include "SDL_render.h"
#include <string>

namespace wbz {
class Map {
public:
  Map() = default;
  Map(const std::string &map_path);
  Map(const std::string &map_path, const SDL_Rect &map_rect);

  void set_map_file(const std::string &map_path);
  void set_map_rect(const SDL_Rect &map_rect);

  void render(SDL_Renderer *renderer);
  void update(float delta_time);

private:
  std::string _texture_id;
  SDL_Rect _map_rect;
};
} // namespace wbz
