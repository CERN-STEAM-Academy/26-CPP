---
title: Benchmark environment — getting interpretable results
author: Dr. Matthias Kretz
date: 0000-01-05
layout: post
---

Before you start optimizing code, you need to make sure your benchmark results are actually meaningful. On a modern 
multi-core server or laptop, the operating system and hardware introduce so much noise that two consecutive runs of the 
exact same binary can differ by 20–30%. This page explains the tools and settings that eliminate that noise.

## Why benchmarks are noisy

Modern CPUs and operating systems do several things that are excellent for everyday usability but terrible for 
micro-benchmarks:

- **CPU frequency scaling** — The kernel adjusts per-core clock speed dynamically (turbo boost, powersave governor). 
  Your benchmark may run at 2.2 GHz on one iteration and 4.8 GHz on the next.
- **Process migration** — The Linux scheduler freely moves your benchmark process between cores. Different cores may 
  have different cache states, different neighboring processes, and even different microarchitectures (e.g. Laptops with 
  performance cores and power-efficient Atom cores).
- **Background activity** — Disk I/O, network interrupts, systemd timers, and other processes compete for CPU time, 
  causing involuntary preemption.
- **Real-time priority inversion** — If any background thread has a higher nice value or scheduling priority than your 
  benchmark, it can steal CPU time mid-measurement.

If you ignore these factors, you will measure the operating system's scheduling behavior in addition to your algorithm's 
performance.

## `taskset` — pin to a specific CPU core

[`taskset`](https://man7.org/linux/man-pages/man1/taskset.1.html) binds a process to a given set of CPU cores, 
preventing the scheduler from migrating it.

```bash
# Pin to core 1 only
taskset -c 1 ./my_benchmark

# Pin to cores 2 and 3
taskset -c 2,3 ./my_benchmark
```

You can also use it as a prefix to any command:

```bash
taskset -c 1 perf stat ./my_benchmark
```

> Why this matters
>
> Without `taskset`, your benchmark process may land on a core sharing a last-level cache with a memory-intensive 
background process, or may migrate mid-run, invalidating warm-cache assumptions. Pinning ensures every iteration sees 
the same hardware topology.
{:.block-tip}

## `chrt` — change scheduling policy and priority

[`chrt`](https://man7.org/linux/man-pages/man1/chrt.1.html) lets you change the Linux scheduling policy of a process. 
The default policy is `SCHED_OTHER` (normal time-sharing). For benchmarks, you typically want `SCHED_FIFO` 
(first-in-first-out real-time):

```bash
# Run with SCHED_FIFO at priority 10 (low RT priority)
chrt -f 10 ./my_benchmark

# Check current policy
chrt -p $$
```

With `SCHED_FIFO`, once your process gets the CPU it keeps it until it voluntarily yields (or a higher-priority RT task 
preempts it). This eliminates involuntary preemption from normal-priority background tasks.

> Priority value
>
> Priorities 99–100 are reserved. Using very high RT priorities (e.g. 90+) for a long-running process can starve the 
entire system and make it unresponsive. For short benchmarks, low RT priorities (1–10) are sufficient and safe.
{:.block-warning}

## `schedtool` — combine both in one command

[`schedtool`](https://github.com/fthiessen/schedtool) wraps both `chrt` and `taskset` into a single invocation. This is 
the tool used in the reference `CMakeLists.txt`:

```bash
schedtool -F -p 10 -a 2 -e ./my_benchmark
```

Breaking down the flags:

| Flag | Meaning |
|---|---|
| `-F` | Use `SCHED_FIFO` scheduling policy |
| `-p 10` | Set real-time priority to 10 |
| `-a 2` | Affinity: bind to CPU core 2 |
| `-e` | Execute the following command (otherwise enters interactive mode) |

This single line gives you RT scheduling, elevated priority, and CPU pinning simultaneously.

> Adjust your `CMakeLists.txt`
>
> Prepend `schedtool -F -p 10 -a 2 -e` to the invocation of the benchmark in `add_benchmark`.
{:.block-task}

> Multithreaded benchmarks
>
> Since the above `schedtool` call restricts the benchmark to a single core, multi-thread benchmarks will execute on a 
> single core. Keep that in mind when you want to play with multi-threading.
>
> Also, if you remove the core restriction, a run-away benchmark can hang your system, as it starves all user processes.
{:.block-warning}

## Disabling CPU frequency scaling — `benchmarking.sh`

Even with `schedtool`, if the CPU changes its clock frequency between iterations, your timing measurements are 
meaningless. You can [download my `benchmarking.sh` script][5].

```bash
# Turn on benchmark mode
./benchmarking.sh on
```

This script performs three actions:

1. **Sets the CPU governor to `performance`** — Writes `performance` to every 
   `/sys/devices/system/cpu/cpufreq/policy*/scaling_governor`. This locks each core at its maximum base frequency.

2. **Disables Turbo Boost** — Depending on the CPU, writes either:
   - `echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo` (Intel)
   - `echo 0 > /sys/devices/system/cpu/cpufreq/boost` (AMD)

3. **Handles permissions** — If needed, uses `sudo` to grant group write access to the sysfs files, so subsequent 
   invocations do not require root.

To restore normal operation:

```bash
# Turn off benchmark mode
/home/mkretz/bin/benchmarking.sh off
```

For convenience, there is an interactive mode:

```bash
# Interactive: press Enter to toggle on/off
/home/mkretz/bin/benchmarking.sh
```

> Why disable turbo boost?
>
> Turbo boost is triggered by temperature, power draw, and active core count. During a benchmark run, the kernel may 
> enable or disable turbo at arbitrary moments depending on thermal headroom. This means identical code can execute at 
> different frequencies across iterations, inflating variance. Disabling turbo ensures every measurement runs at a 
> known, fixed clock rate.
>
> Note: This means your benchmark measures performance at the **base** frequency, not the turbo frequency. For comparing 
> algorithms this is exactly what you want — consistency beats absolute peak numbers.
{:.block-tip}

> When *not* to disable turbo boost
>
> There can be good reasons to keep turbo boost *on*. E.g., if you want to measure best-case throughput for a certain 
> operation / transformation. With lower core frequency, the ratio of memory latency to instruction throughput is skewed 
> in favor of less cache-efficient codes.
>
> On my laptop, the achievable peak FLOP differ by more than a factor 2!
{:.block-warning}

## Putting it all together

For the most reliable benchmark results, combine all layers:

In one terminal window keep interactive `benchmarking.sh` open. You want to turn the benchmark mode off for normal 
interactive usage of the system (faster compiles).

```bash
/home/mkretz/bin/benchmarking.sh
```

And after adding the `schedtool` call to `CMakeLists.txt`, all your benchmarks invoked via the `make run_<name>` target 
automatically use real-time priority without CPU migration.

However, remember to use `schedtool` when invoking manually, e.g. for running in `perf`.

```bash
schedtool -F -p 10 -a 2 -e perf stat ./my_benchmark
```

Or equivalently with separate tools:

```bash
chrt -f 10 taskset -c 2 perf ./my_benchmark
```

## Quick checklist for interpretable benchmarks

> Verify each of these before trusting your numbers:
>
> - [ ] CPU governor is `performance` (check: `cat /sys/devices/system/cpu/cpufreq/policy0/scaling_governor`)
> - [ ] Turbo boost is disabled (check: `cat /sys/devices/system/cpu/intel_pstate/no_turbo` → should be `1`)
> - [ ] Process is pinned to a single performance core (`taskset -cp <pid>`)
> - [ ] Process has RT scheduling (`SCHED_FIFO`)
> - [ ] No heavy background workloads running (check `top`)
> - [ ] Built in `Release` or `RelWithDebInfo` mode (not Debug!)
{:.block-tip}

## References

- [:green_book: Linux `schedtool` man page][1]
- [:green_book: Linux `chrt` man page][2]
- [:green_book: Linux `taskset` man page][3]
- [:green_book: Kernel documentation — CPUFreq sysfs interface][4]

[1]: https://man.archlinux.org/man/schedtool.8.en
[2]: https://man7.org/linux/man-pages/man1/chrt.1.html
[3]: https://man7.org/linux/man-pages/man1/taskset.1.html
[4]: https://docs.kernel.org/admin-guide/pm/cpufreq.html
[5]: assets/benchmarking.sh
