#include <array>
#include <config/config.hpp>
#include <entities/character/character.hpp>
#include <window/window.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// TODO: add assertions
namespace wbz {
class Application {
public:
  static void run();

  static void single_iter(void);

  static Application &instance() {
    static Application application;
    return application;
  }

  const Config &config() const { return _config; }
  bool is_playing() const { return _is_playing; }

private:
  Config _config;
  Window _window;

  std::array<entities::Character, 1> _characters;

  // TODO: Put this in a game state record
  bool _is_playing = true;

  void init();
  void init_characters();
  void handle_events();
  void update();
  void render();
  void cleanup();
};
} // namespace wbz
