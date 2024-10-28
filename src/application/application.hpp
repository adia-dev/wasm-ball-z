#include <config/config.hpp>
#include <managers/game_manager/game_manager.hpp>
#include <state/game_state.hpp>
#include <window/window.hpp>

namespace wbz {
class Application {
public:
  Application() : _game_manager(_game_state) {}

  static void run();
  static void shutdown();
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
  GameState _game_state;
  managers::GameManager _game_manager;

  bool _is_playing = true;
  bool _is_paused = false;

  uint64_t _current_time = 0;
  uint64_t _last_time = 0;
  double _delta_time = 0.0;

  void init();
  void handle_events();
  void update();
  void render();
  void cleanup();

  void toggle_pause();
};
} // namespace wbz
