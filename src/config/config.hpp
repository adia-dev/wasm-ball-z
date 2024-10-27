#pragma once

#include <cstdint>

namespace wbz {

struct WindowConfig {
  uint16_t width = 800;
  uint16_t height = 600;
  const char *title = "Wasm Ball Z";
  uint32_t flags = 0;
};

class Config {
public:
  Config() : _window_config({}), _desired_fps(60) {}

  uint16_t desired_fps() const { return _desired_fps; }
  const WindowConfig &window_config() const { return _window_config; }

private:
  WindowConfig _window_config;
  uint16_t _desired_fps;
};
} // namespace wbz
