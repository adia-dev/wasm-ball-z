#include "SDL_render.h"
#include "SDL_video.h"
#include <SDL.h>
#include <config/config.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace wbz {
class Window {
public:
  static Window from_config(const WindowConfig &window_config) {
    Window new_window;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      std::cerr << "Failed to initialize the SDL2 Library; Error = "
                << SDL_GetError() << std::endl;
      throw std::runtime_error(
          "Failed to initialize SDL with `SDL_INIT_VIDEO` flags.");
    }

    // TODO: initialize the window and the renderer separately to avoid the
    // reinterpret_cast
    int result = SDL_CreateWindowAndRenderer(
        window_config.width, window_config.height, window_config.flags,
        (SDL_Window **)&new_window._window,
        (SDL_Renderer **)&new_window._renderer);

    if (result < 0 || new_window.window() == nullptr ||
        new_window.renderer() == nullptr) {
      std::cerr << "Failed to create the SDL2 Window and Renderer; Error = "
                << SDL_GetError() << std::endl;

      throw std::runtime_error("Failed to create the SDL2 Window and Renderer");
    }

    SDL_SetWindowTitle(new_window._window.get(), window_config.title);
    SDL_SetRenderDrawBlendMode(new_window._renderer.get(), SDL_BLENDMODE_BLEND);

    return new_window;
  }

  SDL_Window *window() { return _window.get(); }
  SDL_Renderer *renderer() { return _renderer.get(); }

private:
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window = {
      nullptr, SDL_DestroyWindow};
  std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _renderer = {
      nullptr, SDL_DestroyRenderer};
};
} // namespace wbz
