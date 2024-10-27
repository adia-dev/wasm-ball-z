#include "application.hpp"

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

  init_characters();

  std::cout << "Successfully initialized the application instance\n";
}

void Application::init_characters() {
  _characters[0].set_name("adia-dev");
  /* _characters[1].set_name("faia-panchi"); */
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
      }
      break;
    }
  }
}

void Application::update() {
  for (entities::Character &character : _characters) {
    character.update(0.0f);
  }
}

void Application::render() {
  SDL_SetRenderDrawColor(_window.renderer(), 0, 0, 0, 255);
  SDL_RenderClear(_window.renderer());

  for (entities::Character &character : _characters) {
    character.render(_window.renderer());
  }

  SDL_RenderPresent(_window.renderer());
}

void Application::cleanup() {
  std::cout << "Cleaning up the application instance\n";
  SDL_DestroyRenderer(_window.renderer());
  SDL_DestroyWindow(_window.window());
  SDL_Quit();

#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif

  std::cout << "Successfully cleaned up the application instance\n";
}
} // namespace wbz
