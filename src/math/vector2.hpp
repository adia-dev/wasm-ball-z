#pragma once

#include <cmath>
#include <ostream>

template <typename T> struct Vector2 {
  Vector2() : x(T()), y(T()) {}
  Vector2(T x, T y) : x(x), y(y) {}
  T x, y;

  template <typename K> Vector2<T> add(const Vector2<K> &rhs) const {
    return Vector2<T>(this->x + (T)rhs.x, this->y + (T)rhs.y);
  };

  template <typename K> Vector2<T> sub(const Vector2<K> &rhs) const {
    return Vector2<T>(this->x - (T)rhs.x, this->y - (T)rhs.y);
  }

  template <typename K> Vector2<T> mul(const Vector2<K> &rhs) const {
    return Vector2<T>(this->x * (T)rhs.x, this->y * (T)rhs.y);
  }

  template <typename K> Vector2<T> mul(K scalar) const {
    return Vector2<T>(this->x * (T)scalar, this->y * (T)scalar);
  }

  template <typename K> Vector2<T> div(const Vector2<K> &rhs) const {
    return Vector2<T>(this->x / (T)rhs.x, this->y / (T)rhs.y);
  }

  // Add a squared magnitude function for performance and avoiding square root
  // when possible
  double mag_squared() const {
    double x_squared =
        static_cast<double>(this->x) * static_cast<double>(this->x);
    double y_squared =
        static_cast<double>(this->y) * static_cast<double>(this->y);
    return x_squared + y_squared;
  }

  template <typename K> Vector2<T> div(K scalar) const {
    // Add protection against division by zero
    if (std::abs(scalar) < 0.0001) {
      return Vector2<T>::zero();
    }
    return Vector2<T>(this->x / (T)scalar, this->y / (T)scalar);
  }

  Vector2<T> normalized() const {
    double magnitude = this->mag();
    // Add protection against division by zero
    if (magnitude < 0.0001) { // Small epsilon value instead of exact zero
      return Vector2<T>::zero();
    }
    return this->div(magnitude);
  }

  double mag() const {
    double x_squared =
        static_cast<double>(this->x) * static_cast<double>(this->x);
    double y_squared =
        static_cast<double>(this->y) * static_cast<double>(this->y);
    return std::sqrt(x_squared + y_squared);
  }

  static Vector2<T> zero() { return Vector2<T>((T)0, (T)0); }

  friend std::ostream &operator<<(std::ostream &os, const Vector2 &v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }
};

typedef Vector2<float> Vector2f;
typedef Vector2<int> Vector2i;
