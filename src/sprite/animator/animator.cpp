#include "animator.hpp"
#include <iostream>

namespace wbz {
Animator::Animator() { play(); }

Animator::~Animator() {}

void Animator::play() { _is_playing = true; }

void Animator::pause() { _is_playing = false; }

void Animator::stop() {
  _is_playing = false;
  reset_animation();
}

void Animator::update(float delta_time) {
  if (!_is_playing || _current_animation_name.empty())
    return;

  try {
    Animation *current_animation = &_animations.at(_current_animation_name);
    _timer += delta_time * 1000;

    if (_timer >= current_animation->delay) {
      _timer -= current_animation->delay;
      _current_animation_frame_index++;

      if (_current_animation_frame_index >= current_animation->frames.size()) {
        if (current_animation->loop) {
          _current_animation_frame_index = 0;
        } else {
          _current_animation_frame_index = current_animation->frames.size() - 1;
          stop();
          if (current_animation->on_complete)
            current_animation->on_complete();
        }
      }
    }
  } catch (const std::out_of_range &) {
    // Handle missing animation gracefully
  }
}

void Animator::add_animation(const std::string &name, Animation animation) {
  if (_animations.count(name)) {
    throw std::invalid_argument("Animation already exists: " + name);
  }
  _animations[name] = animation;
}

const SDL_Rect &Animator::frame() const {
  static SDL_Rect empty_frame{};
  if (_current_animation_name.empty())
    return empty_frame;

  try {
    const Animation *current_animation =
        &_animations.at(_current_animation_name);
    return current_animation->frames[_current_animation_frame_index];
  } catch (const std::out_of_range &) {
    return empty_frame;
  }
}

void Animator::play(const std::string &name) {
  if (_current_animation_name != name) {
    _current_animation_name = name;
    reset_animation();
  }
  play();
}

void Animator::reset_animation() {
  _current_animation_frame_index = 0;
  _timer = 0.0;
}

void Animator::load_animations() {
  // TODO
}

} // namespace wbz
