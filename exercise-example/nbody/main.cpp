// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "nbody_simulation.h"

#include <concepts>

int main()
{
  simulate_to_sfml<float>({
    {{1-0.97000436f, 1.24308753f}, {0.4662036850f, 0.4323657300f}, {0, 0}, 5.f},
    {{1.97000436f, 1-0.24308753f}, {0.4662036850f, 0.4323657300f}, {0, 0}, 2.f},
    {{1, 1}, {-0.93240737f, -0.86473146f}, {0, 0}, 3.f},
    {{1, 2}, {-0.93240737f, -0.86473146f}, {0, 0}, 8.f}
  }, 4000);

  return 0;
}

// vim: et
