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

  template <typename K> Vector2<T> div(K scalar) const {
    return Vector2<T>(this->x / (T)scalar, this->y / (T)scalar);
  }

  Vector2<T> normalized() const {
    double magnitude = this->mag();
    if (magnitude == 0) {
      return Vector2<T>::zero();
    }

    return this->div(magnitude);
  }

  double mag() const {
    return std::sqrt((double)(this->x * this->x + this->y * this->y));
  }

  static Vector2<T> zero() { return Vector2<T>((T)0, (T)0); }

  friend std::ostream &operator<<(std::ostream &os, const Vector2 &v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }
};

typedef Vector2<float> Vector2f;
typedef Vector2<int> Vector2i;
