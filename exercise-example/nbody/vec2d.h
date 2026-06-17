// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>
#include <cmath>

// Abstraction for computing the distance between two points in a linear space
template <std::floating_point T>
  constexpr T
  delta(T a, T b)
  {
    return a - b;
  }

template <typename T>
  struct Vec2d
  {
    T x, y;

    constexpr
    Vec2d() noexcept = default;

    constexpr
    Vec2d(T xx, T yy) noexcept
    : x(xx), y(yy)
    {}

    constexpr
    Vec2d(const Vec2d&) noexcept = default;

    template <typename U>
      requires std::constructible_from<T, U>
      constexpr explicit(not std::convertible_to<U, T>)
      Vec2d(const Vec2d<U>& other) noexcept
      : x(other.x), y(other.y)
      {}

    friend constexpr auto
    distance(Vec2d a, Vec2d b)
    {
      auto dx = delta(a.x, b.x);
      auto dy = delta(a.y, b.y);
      using std::sqrt;
      return sqrt(dx * dx + dy * dy);
    }

    friend constexpr auto
    distance2(Vec2d a, Vec2d b)
    {
      auto dx = delta(a.x, b.x);
      auto dy = delta(a.y, b.y);
      using std::abs;
      return abs(dx * dx + dy * dy);
    }

    friend constexpr bool
    operator==(Vec2d, Vec2d) = default;

    friend constexpr Vec2d
    operator+(Vec2d a, Vec2d b)
    {
      return {a.x + b.x, a.y + b.y};
    }

    friend constexpr Vec2d&
    operator+=(Vec2d& a, Vec2d b)
    {
      return a = a + b;
    }

    friend constexpr auto
    delta(Vec2d a, Vec2d b)
    {
      auto x = delta(a.x, b.x);
      auto y = delta(a.y, b.y);
      return Vec2d<decltype(x)>(x, y);
    }

    friend constexpr Vec2d
    operator-(Vec2d a, Vec2d b)
    {
      return {a.x - b.x, a.y - b.y};
    }

    friend constexpr Vec2d&
    operator-=(Vec2d& a, Vec2d b)
    {
      return a = a - b;
    }

    friend constexpr T
    dot(Vec2d a, Vec2d b)
    {
      return a.x * b.x + a.y * b.y;
    }

    friend constexpr Vec2d
    operator*(T scale, Vec2d a)
    {
      return {scale * a.x, scale * a.y};
    }

    friend constexpr Vec2d
    operator*(Vec2d a, T scale)
    {
      return {scale * a.x, scale * a.y};
    }

    friend constexpr Vec2d&
    operator*=(Vec2d& a, T scale)
    {
      return a = scale * a;
    }

    friend constexpr Vec2d
    operator/(Vec2d a, T divisor)
    {
      return {a.x / divisor, a.y / divisor};
    }
  };

// vim: et
