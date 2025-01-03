#pragma once

#include "SDL_rect.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct Animation {
  float speed_ms = 100;
  float delay = 60;
  std::vector<SDL_Rect> frames;
  bool loop = true;
  std::function<void()> on_complete = nullptr;
};

namespace wbz {
class Animator {
public:
  Animator();
  ~Animator();

  void add_animation(const std::string &name, Animation animation);
  void load_animations();

  void play();
  void play(const std::string &name);
  void pause();
  void stop();

  const SDL_Rect &frame() const;

  void update(float delta_time);

  bool is_playing() const { return _is_playing; }

private:
  bool _is_playing = false;
  int _current_animation_frame_index = 0;
  float _timer = 0.0;

  std::string _current_animation_name = "";
  std::unordered_map<std::string, Animation> _animations;

  void reset_animation();
};
} // namespace wbz
