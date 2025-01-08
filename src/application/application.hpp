#pragma once

#include <config/config.hpp>
#include <iostream>
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

  static void toggle_headless() {
    auto &app = instance();
    app._headless = !app._headless;
    std::cout << "Headless mode: " << (app._headless ? "ON" : "OFF")
              << std::endl;
  }

private:
  Config _config;
  Window _window;
  GameState _game_state;
  managers::GameManager _game_manager;

  bool _is_playing = true;
  bool _is_paused = false;

  bool _headless = false;

  uint64_t _current_time = 0;
  uint64_t _last_time = 0;
  double _delta_time = 0.0;
  float _camera_scale = 1.0f;
  float _camera_target_scale = 1.0f;
  float _min_scale = 0.5f;
  float _max_scale = 1.2f;

  float _camera_x_offset = 0.0f;
  float _camera_y_offset = 0.0f;

  void init();
  void handle_events();
  void update();
  void update_camera(double delta_time);
  void render();
  void cleanup();

  void toggle_pause();
  void update_game_state(double delta_time);
};
} // namespace wbz
