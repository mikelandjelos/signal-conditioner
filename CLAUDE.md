# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Quick reference

```bash
# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel

# Debug + ASan
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON && cmake --build build-asan --parallel

# Run
./build/virt_sig_cond --dataset data/imu0/data.csv --cutoff 40 --output output.csv

# Tests
cd build && ctest --output-on-failure

# Format check (what the pre-commit hook runs)
clang-format --dry-run --Werror include/*.hpp src/*.cpp tests/*.cpp

# Format in-place
clang-format -i include/*.hpp src/*.cpp tests/*.cpp

# Valgrind (plain Debug build — not ASan, they conflict)
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build-debug --parallel
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 \
  ./build-debug/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 \
  ./build-debug/tests/virt_sig_cond_tests

# Doxygen API docs (output: docs/doxygen/html/index.html)
cmake --build build --target docs
```

Detailed docs live in `docs/` — read those before making structural changes:

| Doc | Contents |
|-----|----------|
| `docs/01-project-setup.md` | Prerequisites, tooling config, dataset setup |
| `docs/02-development-workflow.md` | Build variants, running the app, plotting |
| `docs/03-testing-workflow.md` | GTest, ctest, disabled tests, acceptance targets |
| `docs/04-profiling-workflow.md` | ASan, Valgrind, perf, future report scaffold |

## CI (GitHub Actions)

| Workflow | File | Gate? |
|----------|------|-------|
| Format + Build + Tests | `.github/workflows/ci.yml` | Hard — blocks merge |
| ASan/UBSan + Valgrind | `.github/workflows/profiling.yml` | Hard — blocks merge |
| Timing smoke test | `.github/workflows/profiling.yml` (`timing-informational`) | Informational only (`continue-on-error: true`) |

Timing non-regression in CI is informational because shared runners have too much jitter to enforce the ±0.5 ms criterion. Validate timing locally with the real EuRoC dataset.

## Project goal

Hardware-free C++17 command-line pipeline that replays a public IMU dataset (EuRoC/KITTI) at true sensor rate, identifies noise spectrum via a self-written FFT, and attenuates vibration with a hand-derived Butterworth IIR filter. See `project1.pdf` for the full specification.

## Hard constraints (non-negotiable acceptance criteria)

- **No external DSP libraries.** FFT, Hann window, Butterworth coefficients, and Direct-Form I must all be hand-implemented. SciPy/MATLAB are allowed only for reference verification, not in the build.
- **`sleep_until`, never `sleep_for`.** The timing loop uses `std::chrono::steady_clock` + `sleep_until(deadline)`. Drift accumulates into seconds with `sleep_for`.
- **Pre-warp the cutoff** before the bilinear transform: `Ωc = (2/T)·tan(ωc·T/2)`. Skipping this shifts the digital cutoff.
- **Hann window the FFT input.** Rectangular windowing causes spectral leakage mistaken for noise peaks.
- **Zero steady-state heap allocation.** The hot loop must not allocate. Verified with Valgrind/ASan.
- **Streaming ingestion.** `LogReader` reads line-by-line — never loads the whole file.
- **Runtime-configurable cutoff and order** via CLI (`--cutoff`, future `--order`).

## Architecture

`FFTProfiler<N>` is a header-only template (`include/fft.hpp`) — all sizes compile wherever the header is included; no explicit instantiation needed. All other modules split into `include/*.hpp` + `src/*.cpp`.

The library target `virt_sig_cond_lib` (static) is shared between the main binary and the test binary, so implementation files are compiled once.

GTest is fetched via `FetchContent` and cached in `.cmake-deps/` (gitignored, persists across build-dir wipes).

## Grading weight (prioritise accordingly)

| Criterion | Pts |
|-----------|-----|
| Mathematical accuracy of the IIR filter | 35 |
| Loop determinism & timing accuracy | 25 |
| Memory & systems discipline | 20 |
| Analytical verification & logging | 20 |
