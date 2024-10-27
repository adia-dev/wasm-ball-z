#include "window.hpp"
#include <iostream>
#include <stdexcept>

wbz::Window wbz::Window::from_config(const WindowConfig &window_config) {
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

void wbz::Window::cleanup() {
  SDL_DestroyWindow(_window.get());
  SDL_DestroyRenderer(_renderer.get());

  _window = nullptr;
  _renderer = nullptr;
}
