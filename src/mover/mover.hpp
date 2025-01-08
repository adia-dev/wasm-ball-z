#pragma once

#include <SDL_rect.h>
#include <math/vector2.hpp>

namespace wbz {
class Mover {
public:
  explicit Mover(float mass = 1.0f, const Vector2f &position = Vector2f::zero())
      : _mass(mass), _acceleration(Vector2f::zero()),
        _velocity(Vector2f::zero()), _position(position) {}

  void set_position(const Vector2f &position) { _position = position; }
  void set_velocity(const Vector2f &velocity) { _velocity = velocity; }

  const Vector2f &acceleration() const { return _acceleration; }
  const Vector2f &position() const { return _position; }
  const Vector2f &velocity() const { return _velocity; }

  void set_mass(float mass) { _mass = mass; }
  float mass() const { return _mass; }

  void add_force(const Vector2f &force) {
    _acceleration = _acceleration.add(force.div(_mass == 0.0f ? 1.0f : _mass));
  }

  void update(double delta_time) {

    if (delta_time <= 0.0 || std::isnan(delta_time)) {
      return;
    }

    if (std::isnan(_acceleration.x) || std::isnan(_acceleration.y)) {
      _acceleration = Vector2f::zero();
    }

    Vector2f new_velocity = _velocity.add(_acceleration.mul(delta_time));
    if (!std::isnan(new_velocity.x) && !std::isnan(new_velocity.y)) {
      _velocity = new_velocity;
    }

    Vector2f new_position = _position.add(_velocity.mul(delta_time));
    if (!std::isnan(new_position.x) && !std::isnan(new_position.y)) {
      _position = new_position;
    }

    _acceleration = Vector2f::zero();
  }

private:
  float _mass;
  Vector2f _acceleration, _velocity, _position;
};
} // namespace wbz
