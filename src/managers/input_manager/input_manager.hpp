#pragma once

#include <cstdint>
#include <unordered_map>

#include "SDL_events.h"

namespace wbz {

enum class InputState {
  NOT_PRESSED = 0,
  PRESSED,
  DOWN,
  RELEASED,
};

namespace managers {
class InputManager {
public:
  static InputManager &instance() {
    static InputManager instance;
    return instance;
  }

  static void handle_events(const SDL_Event &event) {
    if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
      return;
    }

    auto &app = instance();

    if (event.type == SDL_KEYDOWN &&
        app._key_states[event.key.keysym.sym] == InputState::PRESSED) {
      app._key_states[event.key.keysym.sym] = InputState::DOWN;
    } else if (event.type == SDL_KEYDOWN) {
      app._key_states[event.key.keysym.sym] = InputState::PRESSED;
    } else {
      app._key_states[event.key.keysym.sym] = InputState::RELEASED;
    }
  }

  static void update() {
    auto &app = instance();
    for (auto &[key, state] : app._key_states) {
      switch (state) {
      case InputState::RELEASED:
        state = InputState::NOT_PRESSED;
      case InputState::PRESSED:
      case InputState::DOWN:
      case InputState::NOT_PRESSED:
        break;
      }
    }
  }

  static bool is_key_pressed(uint16_t key) {
    auto &app = instance();
    if (!app._key_states.count(key))
      return false;

    return app._key_states.at(key) == InputState::PRESSED;
  }

  static bool is_key_down(uint16_t key) {
    auto &app = instance();
    if (!app._key_states.count(key))
      return false;

    return app._key_states.at(key) == InputState::DOWN;
  }

  static bool is_key_released(uint16_t key) {
    auto &app = instance();
    if (!app._key_states.count(key))
      return false;

    return app._key_states.at(key) == InputState::RELEASED;
  }

private:
  InputManager() = default;
  std::unordered_map<uint16_t, InputState> _key_states;

  InputManager(const InputManager &) = delete;
  InputManager &operator=(const InputManager &) = delete;
};
} // namespace managers
} // namespace wbz
