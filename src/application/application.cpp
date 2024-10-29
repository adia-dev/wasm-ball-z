#include "application.hpp"
#include "SDL_render.h"

#include <iostream>
#include <managers/input_manager/input_manager.hpp>
#include <managers/resource_manager/resource_manager.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace wbz {

void Application::run() {
  auto &app = instance();
  app.init();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(Application::single_iter, 0, 1);
#endif

  while (app.is_playing()) {
    app.handle_events();
    app.update();
    app.render();
  }

  app.cleanup();
}

void Application::shutdown() {
  auto &app = instance();
  app.cleanup();
}

void Application::single_iter(void) {
  auto &app = instance();
  app.handle_events();
  app.update();
  app.render();

  if (!app.is_playing()) {
    app.cleanup();
  }
}

void Application::init() {
  std::cout << "Initializing the application instance\n";
  _window = Window::from_config(_config.window_config());

  _last_time = SDL_GetPerformanceCounter();

  _game_manager.init();

  std::cout << "Successfully initialized the application instance\n";
}

void Application::handle_events() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      _is_playing = false;
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {
      case SDLK_ESCAPE:
        _is_playing = false;
        break;
      case SDLK_p:
        toggle_pause();
        break;
      }
      break;
    }

    managers::InputManager::handle_events(e);
  }
}

void Application::update() {
  _current_time = SDL_GetPerformanceCounter();
  _delta_time = (_current_time - _last_time) /
                static_cast<double>(SDL_GetPerformanceFrequency());
  _last_time = _current_time;

  if (_is_paused) {
    return;
  }

  for (auto &entity : _game_state.entities) {
    entity->update(_delta_time);
  }

  _game_manager.update();

  managers::InputManager::update();
}

void Application::render() {
  SDL_SetRenderDrawColor(_window.renderer().get(), 0, 0, 0, 255);
  SDL_RenderClear(_window.renderer().get());

  for (const auto &entity : _game_state.entities) {
    entity->render(_window.renderer().get());
  }

  SDL_RenderPresent(_window.renderer().get());
}

void Application::cleanup() {
  std::cout << "Cleaning up the application instance\n";

  _window.cleanup();
  _game_manager.cleanup();

  SDL_Quit();

  std::cout << "Successfully cleaned up the application instance\n";

#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
}

void Application::toggle_pause() {
  _is_paused = !_is_paused;
  std::cout << "Game " << (_is_paused ? "paused" : "resumed") << std::endl;
}
} // namespace wbz
