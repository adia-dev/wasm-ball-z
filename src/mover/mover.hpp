#pragma once

#include <math/vector2.hpp>

namespace wbz {
class Mover {
public:
  explicit Mover(float mass) : _mass(mass) {}
  void set_position(const Vector2f &position) { _position = position; }

  const Vector2f &acceleration() const { return _acceleration; }
  const Vector2f &position() const { return _position; }
  const Vector2f &velocity() const { return _velocity; }

  void set_mass(float mass) { _mass = mass; }
  const float &mass() const { return _mass; }

  void add_force(const Vector2f &force) {
    _acceleration = _acceleration.add(force.div(_mass));
  }

  void update(double delta_time) {
    _velocity = _velocity.add(_acceleration.mul(delta_time));
    _position = _position.add(_velocity.mul(delta_time));
    _acceleration = Vector2f::zero();

    // Bouncing on the walls
    if (_position.x < 0) {
      _position.x = 0;
      _velocity.x = -_velocity.x;
    }
    if (_position.x >= 800 - (_mass * 5.0f)) {
      _position.x = 800 - (_mass * 5.0f);
      _velocity.x = -_velocity.x;
    }

    /*     if (_position.y < 0) { */
    /*       _position.y = 0; */
    /*       _velocity.y = -_velocity.y; */
    /*     } */

    if (_position.y >= 600 - (_mass * 5.0f)) {
      _position.y = 600 - (_mass * 5.0f);
      _velocity.y = -_velocity.y;
    }
  }

private:
  float _mass;
  Vector2f _acceleration, _velocity, _position;
};
} // namespace wbz
