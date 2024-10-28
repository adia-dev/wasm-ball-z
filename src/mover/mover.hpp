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

  const Vector2f &acceleration() const { return _acceleration; }
  const Vector2f &position() const { return _position; }
  const Vector2f &velocity() const { return _velocity; }

  void set_mass(float mass) { _mass = mass; }
  float mass() const { return _mass; }

  void add_force(const Vector2f &force) {
    _acceleration = _acceleration.add(force.div(_mass == 0.0f ? 1.0f : _mass));
  }

  void update(double delta_time) {
    _velocity = _velocity.add(_acceleration.mul(delta_time));
    _position = _position.add(_velocity.mul(delta_time));
    _acceleration = Vector2f::zero();
  }

private:
  float _mass;
  Vector2f _acceleration, _velocity, _position;
};
} // namespace wbz
