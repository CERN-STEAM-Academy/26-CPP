// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2026      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                       Matthias Kretz <m.kretz@gsi.de>

#include "body.h"

#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <optional>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

constexpr float epsilon2 = 1.e-04f;

// universal gravitational constant
constexpr float G = 1;// 6.67e-11;

template <typename T>
  constexpr void
  update_positions(const T dt, std::span<Body<T>> bodies)
  {
    for (auto& [r, v, a, m] : bodies)
      r += v * dt + a / 2 * (dt * dt);
  }

template <typename T>
  constexpr void
  update_a_and_v(const T dt, std::span<Body<T>> bodies)
  {
    for (std::size_t i = 0; i < bodies.size(); ++i)
      {
        const auto& [ri, vi, ai, mi] = bodies[i];
        Vec2d<T> a = {};
        for (std::size_t j = 0; j < bodies.size(); ++j)
          {
            if (i == j) continue;
            const auto& [rj, vj, aj, mj] = bodies[j];
            using std::pow;
            a -= mj / pow(distance2(ri, rj) + epsilon2, 1.5f) * delta(ri, rj);
          }
        a *= G;
        bodies[i].v += (ai + a) * (dt / 2);
        bodies[i].a = a;
      }
  }

template <typename T>
  constexpr void
  do_timestep(T dt, std::span<Body<T>> bodies)
  {
    update_positions(dt, bodies);
    update_a_and_v(dt, bodies);
  }

template <typename T>
  constexpr T
  kinetic_energy(std::span<const Body<T>> bodies)
  {
    T k = 0;
    for (const auto& [_, v, _, m] : bodies)
      k += m / 2 * dot(v, v);
    return k;
  }

template <typename T>
  constexpr T
  potential_energy(std::span<const Body<T>> bodies)
  {
    T u = 0;
    for (std::size_t i = 0; i < bodies.size(); ++i)
      {
        const auto& [ri, _, _, mi] = bodies[i];
        for (std::size_t j = i + 1; j < bodies.size(); ++j)
          {
            const auto& [rj, _, _, mj] = bodies[j];
            u -= mi * mj / distance(ri, rj);
          }
      }
    return u * G;
  }

template <typename T>
  constexpr void
  simulate_to_csv(std::vector<Body<T>> bodies, const int steps)
  {
    constexpr T dt = 0.001;
    for (int j = 0; j < steps; ++j)
      {
        do_timestep<T>(dt, bodies);
        print_positions_csv<T>(bodies, kinetic_energy<T>(bodies) + potential_energy<T>(bodies));
      }
  }

template <typename T>
  constexpr void
  simulate_to_sfml(std::vector<Body<T>> bodies, const int steps)
  {
    sf::RenderWindow window(sf::VideoMode({1000, 1000}), "Window");
    constexpr T dt = 0.0001;
    while (window.isOpen())
      {
        while (const std::optional event = window.pollEvent())
          { // Close window: exit
            if (event->is<sf::Event::Closed>())
              window.close();
          }
        window.clear();
        for (const auto& [r, ..._, m] : bodies)
          {
            sf::CircleShape point(m);
            float x = r.x.to_float();
            x *= 1000 / r.x.length;
            float y = r.y.to_float();
            y *= 1000 / r.y.length;
            point.setPosition({x, y});
            window.draw(point);
          }
        window.display();
        do_timestep<T>(dt, bodies);

        //using namespace std::literals;
        //std::this_thread::sleep_for(0.5ms);
        //print_positions_csv<T>(bodies, kinetic_energy<T>(bodies) + potential_energy<T>(bodies));
      }
  }



// vim: et
