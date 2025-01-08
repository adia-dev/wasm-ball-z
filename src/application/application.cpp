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

      case SDLK_h:
        toggle_headless();
        break;
      }
      break;
    }

    wbz::managers::InputManager::handle_events(e);
  }
}

void Application::update() {
  size_t nIter = _headless ? 100000 : 1;
  for (size_t i = 0; i < nIter; ++i) {
    _current_time = SDL_GetPerformanceCounter();
    _delta_time = (_current_time - _last_time) /
                  static_cast<double>(SDL_GetPerformanceFrequency());
    _last_time = _current_time;

    if (_is_paused) {
      return;
    }

    update_camera(_delta_time);

    for (auto &entity : _game_state.entities) {
      entity->update(_delta_time);
    }
    _game_state.map.update(_delta_time);

    _game_manager.update(_delta_time);
    managers::InputManager::update();
  }
}

void Application::update_camera(double delta_time) {
  if (!_game_state.player_character) {
    return;
  }

  Vector2f playerPos = _game_state.player_character->mover().position();
  Vector2f aiPos;
  for (auto &ent : _game_state.entities) {
    auto c = std::dynamic_pointer_cast<entities::Character>(ent);
    if (c && c != _game_state.player_character) {
      aiPos = c->mover().position();
      break;
    }
  }

  Vector2f midpoint =
      Vector2f((playerPos.x + aiPos.x) * 0.5f, (playerPos.y + aiPos.y) * 0.5f);

  float dist = playerPos.sub(aiPos).mag();

  float desired = 1.0f - (dist - 300.0f) / 300.0f;
  // clamp
  if (desired < _min_scale)
    desired = _min_scale;
  if (desired > _max_scale)
    desired = _max_scale;

  _camera_target_scale = desired;

  float alpha = 3.0f * static_cast<float>(delta_time);
  _camera_scale += (_camera_target_scale - _camera_scale) * alpha;

  float screenW = (float)_config.window_config().width;
  float screenH = (float)_config.window_config().height;

  _camera_x_offset = (screenW * 0.5f) / _camera_scale - midpoint.x;
  _camera_y_offset = (screenH * 0.5f) / _camera_scale - midpoint.y;
}

void Application::render() {
  if (_headless)
    return;

  auto renderer = _window.renderer().get();

  // SDL_RenderSetScale(renderer, _camera_scale, _camera_scale);

  // SDL_Rect viewport;
  // viewport.x = static_cast<int>(_camera_x_offset * _camera_scale);
  // viewport.y = static_cast<int>(_camera_y_offset * _camera_scale);
  // viewport.w = _config.window_config().width;
  // viewport.h = _config.window_config().height;
  // SDL_RenderSetViewport(renderer, &viewport);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  _game_state.map.render(renderer);
  for (auto &entity : _game_state.entities) {
    entity->render(renderer);
  }

  SDL_RenderPresent(renderer);

  SDL_RenderSetScale(renderer, 1.0f, 1.0f);
  SDL_RenderSetViewport(renderer, nullptr);
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
