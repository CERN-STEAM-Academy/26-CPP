// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "vec2d.h"

#include <print>

// A value from [0, Length)
template <std::floating_point auto Length>
struct InfSpace
{
  using FpType = decltype(Length);

  static constexpr FpType length = Length;

  static constexpr FpType scale
    = Length / std::bit_floor(std::numeric_limits<unsigned>::max()) / 2;

  unsigned value;

  constexpr explicit
  operator FpType() const
  { return scale * value; }

  constexpr FpType
  to_float() const
  { return scale * value; }

  constexpr
  InfSpace(FpType x)
  : value([=] mutable {
      while (x < 0) x += Length;
      while (x >= Length) x -= Length;
      return x;
    }() / scale)
  {}

  friend constexpr bool
  operator==(InfSpace, InfSpace) = default;

  friend constexpr FpType
  delta(InfSpace a, InfSpace b)
  {
    unsigned d = a.value - b.value;
    if (d < -d)
      return scale * d;
    else
      return -scale * -d;
  }

  friend constexpr InfSpace
  operator+(InfSpace a, InfSpace b)
  {
    a.value += b.value;
    return a;
  }

  friend constexpr InfSpace
  operator-(InfSpace a)
  {
    a.value = -a.value;
    return a;
  }
};

template <typename T>
  struct Body
  {
    //Vec2d<T> r;
    Vec2d<InfSpace<T(3.5)>> r;
    Vec2d<T> v, a;
    T m;
  };

template <typename T>
  void print_body(const Body<T>& b)
  {
    const auto& [r, v, a, m] = b;
    std::println("r=({}, {}), v=({}, {}), a=({}, {}), m={}", T(r.x), T(r.y), v.x, v.y, a.x, a.y, m);
  }

template <typename T>
  void print_state(std::span<const Body<T>> bodies)
  {
    for (const auto& b : bodies)
      print_body(b);
  }

template <typename T>
  void print_positions_csv(std::span<const Body<T>> bodies, T energy)
  {
    for (const auto& [r, ..._] : bodies)
      std::print("{}, {}, ", T(r.x), T(r.y));
    std::println("{}", energy);
  }


// vim: et
