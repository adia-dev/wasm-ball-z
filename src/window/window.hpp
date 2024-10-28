#pragma once

#include <SDL.h>

#include <config/config.hpp>
#include <memory>

namespace wbz {
class Window {
public:
  static Window from_config(const WindowConfig &window_config);

  std::shared_ptr<SDL_Window> window() { return _window; }
  std::shared_ptr<SDL_Renderer> renderer() { return _renderer; }

  void cleanup();

private:
  std::shared_ptr<SDL_Window> _window{nullptr, SDL_DestroyWindow};
  std::shared_ptr<SDL_Renderer> _renderer{nullptr, SDL_DestroyRenderer};
};
} // namespace wbz
