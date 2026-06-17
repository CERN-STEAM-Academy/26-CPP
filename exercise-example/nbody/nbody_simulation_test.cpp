// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "nbody_simulation.h"
#include "verify.h"

consteval
{ // No bodies / no energy
  std::vector<Body<float>> b0 = {};
  VERIFY(potential_energy<float>(b0) == 0);
  VERIFY(kinetic_energy<float>(b0) == 0);
  do_timestep<float>(0.001, b0);
  VERIFY(potential_energy<float>(b0) == 0);
  VERIFY(kinetic_energy<float>(b0) == 0);
}

consteval
{ // One body moving along x
  std::vector<Body<float>> b1 = {{{1, 1}, {1, 0}, {}, 1}};
  VERIFY(potential_energy<float>(b1) == 0);
  VERIFY(kinetic_energy<float>(b1) == 0.5);
  do_timestep<float>(1, b1);
  VERIFY(potential_energy<float>(b1) == 0);
  VERIFY(kinetic_energy<float>(b1) == 0.5);
  VERIFY(b1[0].r == Vec2d(2.f, 1.f));
}

consteval
{ // Two bodies at rest — verifies gravitational attraction creates opposing velocities
  std::vector<Body<float>> b2 = {
    {{0, 0}, {}, {}, 1},
    {{1, 0}, {}, {}, 1},
  };
  VERIFY(potential_energy<float>(b2) < 0);
  VERIFY(kinetic_energy<float>(b2) == 0);
  do_timestep<float>(0.001f, b2);
  VERIFY(kinetic_energy<float>(b2) > 0);
  VERIFY(b2[0].a.x > 0);
  VERIFY(b2[1].a.x < 0);
  VERIFY(b2[0].a.y == 0);
  VERIFY(b2[1].a.y == 0);
  VERIFY(b2[0].v.x > 0);
  VERIFY(b2[1].v.x < 0);
  VERIFY(b2[0].v.y == 0);
  VERIFY(b2[1].v.y == 0);
}

consteval
{ // Three-body system — verifies multi-body potential/kinetic energy
  std::vector<Body<float>> b3 = {
    {{0, 0}, {}, {}, 1},
    {{1, 0}, {}, {}, 1},
    {{0, 1}, {}, {}, 1},
  };
  VERIFY(potential_energy<float>(b3) < 0);
  VERIFY(kinetic_energy<float>(b3) == 0);
  do_timestep<float>(0.001f, b3);
  VERIFY(kinetic_energy<float>(b3) > 0);
}

consteval
{ // Symmetric two-body with initial velocity — checks symmetry preservation and energy conservation
  // over one timestep
  std::vector<Body<float>> bsym = {
    {{-0.9f, 0}, {0, 0.1f}, {}, 1},
    {{0.9f, 0}, {0, -0.1f}, {}, 1},
  };
  float E0 = kinetic_energy<float>(bsym) + potential_energy<float>(bsym);
  do_timestep<float>(0.01f, bsym);
  float E1 = kinetic_energy<float>(bsym) + potential_energy<float>(bsym);
  VERIFY(E1 >= E0 - 0.01f);
  VERIFY(E1 <= E0 + 0.01f);
  //VERIFY(bsym[0].r.x == -bsym[1].r.x);
  //VERIFY(bsym[0].r.y == -bsym[1].r.y);
  VERIFY(bsym[0].v.y == -bsym[1].v.y);
}

consteval
{ // Single body with mass=2 and diagonal velocity — verifies position update and constant kinetic
  // energy
  std::vector<Body<float>> bm = {
    {{0, 0}, {1, 1}, {}, 2},
  };
  VERIFY(kinetic_energy<float>(bm) == 2);
  do_timestep<float>(1, bm);
  VERIFY(bm[0].r == Vec2d(1.f, 1.f));
  VERIFY(kinetic_energy<float>(bm) == 2);
}

consteval
{ // Two-body energy conservation over 100 timesteps — checks total energy stays bounded and bodies
  // move toward each other
  std::vector<Body<float>> bgrav = {
    {{0, 0}, {}, {}, 1},
    {{1.5f, 0}, {}, {}, 1},
  };
  float E0 = kinetic_energy<float>(bgrav) + potential_energy<float>(bgrav);
  for (int i = 0; i < 100; ++i)
    do_timestep<float>(0.001f, bgrav);
  float E1 = kinetic_energy<float>(bgrav) + potential_energy<float>(bgrav);
  VERIFY(E1 >= E0 - 0.01f);
  VERIFY(E1 <= E0 + 0.01f);
  VERIFY(float(bgrav[0].r.x) > 0.f);
  VERIFY(float(bgrav[1].r.x) < 1.5f);
}
