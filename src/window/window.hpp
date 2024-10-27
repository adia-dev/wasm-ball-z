#pragma once

#include <SDL.h>
#include <config/config.hpp>
#include <memory>

namespace wbz {
class Window {
public:
  static Window from_config(const WindowConfig &window_config);

  SDL_Window *window() { return _window.get(); }
  SDL_Renderer *renderer() { return _renderer.get(); }

  void cleanup();

private:
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window = {
      nullptr, SDL_DestroyWindow};
  std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _renderer = {
      nullptr, SDL_DestroyRenderer};
};
} // namespace wbz
