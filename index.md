---
layout: home
title: Tutorial Material
permalink: /
---

Welcome to the **C++ course** of the CERN STEAM Academy 2026, taught by Matthias Kretz.

The course runs locally on the academy laptops. This page collects the course materials. For the general computing environment (accounts, WiFi, editors, lxplus, AFS, CERNBox) see the [STEAM Academy documentation](https://stac.docs.cern.ch/).

This page provides the material supporting the hands-on work of the C++
tutorial at the CERN STEAM Academy '26

## Toolchain on the laptops

- **Compilers** (environment modules): `module load gcc/16.1.0` or `module load clang/22.1.5`.
- **Google Benchmark** and **SFML 3.1** are pulled in automatically by the example's CMake (`FetchContent`); the build prerequisites are preinstalled on the laptops.
- **Benchmarking:** `schedtool` / `chrt` / `taskset` for real-time scheduling without `sudo`, `perf` counters enabled, and a writable CPU governor — see the [environment docs](https://stac.docs.cern.ch/).

## Schedule

The programme is on the [Indico timetable](https://indico.cern.ch/event/1697464/timetable).

## Exercises

<ul class="summary">
  {% assign home_toc = site.posts | reverse %}
  {% for post in home_toc %}
    <li class="chapter" data-level="1.1" data-path="{{site.baseurl}}{{post.url}}">
    <a href="{{site.baseurl}}{{post.url}}">{{ post.title | escape }}</a>
    </li>
  {% endfor %}
</ul>

<!--[Slides](assets/usingcpp-simd-slides.pdf)-->

Copyright © 2024–2026 GSI / Matthias Kretz.

License: CC BY-NC-SA 4.0
