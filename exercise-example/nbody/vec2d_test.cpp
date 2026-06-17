// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "vec2d.h"
#include "verify.h"

consteval
{
  Vec2d<float> a = {};
  VERIFY(a.x == 0);
  VERIFY(a.y == 0);

  VERIFY(2 * a == a);
  VERIFY(a * 2 == a);

  Vec2d<float> b = {1, 2};
  VERIFY(a + b == b);

  Vec2d<float> b2 = {2, 4};
  VERIFY(2 * b == b2);
  VERIFY(b * 2 == b2);
  VERIFY(b + b == b2);

  b2 -= b;
  VERIFY(b2 == b);
  b2 += b;
  VERIFY(b2 == b + b);

  b2 *= 2;
  VERIFY(b2 == 4 * b);

  VERIFY(b2 / 4 == b);

  VERIFY(dot(a, b) == 0);
  VERIFY(dot(b, b) == 5);

  Vec2d<float> c = {3, 4};
  VERIFY(distance2(a, c) == 25);
#ifndef __clang__
  // Clang can't do constexpr sqrt :-(
  VERIFY(distance(a, c) == 5);
#endif
}
