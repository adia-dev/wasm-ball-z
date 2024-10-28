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

  new_window._window.reset(
      SDL_CreateWindow(window_config.title, SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, window_config.width,
                       window_config.height, window_config.flags),
      SDL_DestroyWindow);

  if (!new_window._window) {
    std::cerr << "Failed to create the SDL2 Window; Error = " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    throw std::runtime_error("Failed to create the SDL2 Window.");
  }

  new_window._renderer.reset(
      SDL_CreateRenderer(new_window._window.get(), -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
      SDL_DestroyRenderer);

  if (!new_window._renderer) {
    std::cerr << "Failed to create the SDL2 Renderer; Error = "
              << SDL_GetError() << std::endl;
    new_window.cleanup();
    SDL_Quit();
    throw std::runtime_error("Failed to create the SDL2 Renderer.");
  }

  if (SDL_SetRenderDrawBlendMode(new_window._renderer.get(),
                                 SDL_BLENDMODE_BLEND) < 0) {
    std::cerr << "Failed to set SDL Renderer blend mode; Error = "
              << SDL_GetError() << std::endl;
    new_window.cleanup();
    SDL_Quit();
    throw std::runtime_error("Failed to set SDL Renderer blend mode.");
  }

  return new_window;
}

void wbz::Window::cleanup() {
  if (_renderer) {
    SDL_DestroyRenderer(_renderer.get());
    _renderer = nullptr;
  }

  if (_window) {
    SDL_DestroyWindow(_window.get());
    _window = nullptr;
  }

  SDL_Quit();
}
