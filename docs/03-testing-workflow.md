# Testing Workflow

## Framework

[GoogleTest v1.15.2](https://github.com/google/googletest) — fetched via CMake `FetchContent` on first configure. No manual install required.

## Building tests

Tests are built alongside the main binary by default (`BUILD_TESTS=ON`). To skip them:

```bash
cmake -B build -DBUILD_TESTS=OFF
```

## Running all tests

```bash
cd build && ctest --output-on-failure
```

Or run the test binary directly for richer output:

```bash
./build/tests/virt_sig_cond_tests
```

## Running a single test suite or test case

GTest's `--gtest_filter` flag accepts glob patterns:

```bash
# All filter tests
./build/tests/virt_sig_cond_tests --gtest_filter="ButterworthFilter.*"

# One specific test
./build/tests/virt_sig_cond_tests --gtest_filter="LogReader.OpensValidFile"

# Everything except player tests
./build/tests/virt_sig_cond_tests --gtest_filter="-RealTimePlayer.*"
```

Via ctest:

```bash
cd build && ctest -R ButterworthFilter --output-on-failure
```

## Test structure

| File | What it tests |
|------|---------------|
| `tests/test_filter.cpp` | Coefficient correctness, DC pass, attenuation, reset |
| `tests/test_fft.cpp` | Buffer fill, ready flag, silence spectrum, frequency detection |
| `tests/test_ingestion.cpp` | File open, header skip, single/multi-row CSV parsing |
| `tests/test_player.cpp` | Callback count, stats accumulation, timing stddev |

### Disabled tests

Tests that exercise unimplemented stubs are prefixed `DISABLED_` (GTest convention — they are registered but not run). When you implement a module, remove the `DISABLED_` prefix to re-enable:

```
DISABLED_PassesDC                    → PassesDC
DISABLED_DetectsDominantFrequency    → DetectsDominantFrequency
DISABLED_ParsesSingleRow             → ParsesSingleRow
DISABLED_ParsesMultipleRows          → ParsesMultipleRows
DISABLED_InvokesCallbackForEverySample → InvokesCallbackForEverySample
DISABLED_StatsReflectSampleCount     → StatsReflectSampleCount
```

## Running tests under AddressSanitizer

Use the `build-asan` variant — ASan is linked into the test binary automatically:

```bash
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-asan --parallel
cd build-asan && ctest --output-on-failure
```

Any memory error causes the test to abort immediately with a full stack trace.

## Adding a new test

1. Add a `TEST(Suite, CaseName)` block to the appropriate `tests/test_*.cpp` file.
2. If it tests an unimplemented feature, prefix the case name with `DISABLED_`.
3. Rebuild — `gtest_discover_tests` picks it up automatically, no CMake edit needed.

## CI behaviour

The `ci.yml` workflow runs the full test suite on every push and pull request to `main` — it is a hard merge gate. The `profiling.yml` workflow additionally runs tests under ASan/UBSan and Valgrind; the timing job is informational only (`continue-on-error: true`). See `docs/04-profiling-workflow.md` for the rationale.

## Acceptance targets (definition of done)

These are the tests that must pass before the project is considered complete:

| Test | Criterion |
|------|-----------|
| `ButterworthFilter.PassesDC` | Output ≈ 1.0 after settling on constant 1.0 input |
| `ButterworthFilter.AttenuatesAboveCutoff` | Peak < 0.25 (−12 dB) at 90 Hz with 40 Hz cutoff |
| `FFTProfiler.DetectsDominantFrequency` | Top bin within 2 bins of injected test frequency |
| `LogReader.ParsesSingleRow` | Parsed values match written values to 1e-9 |
| `LogReader.ParsesMultipleRows` | All N rows parsed in order |
| `RealTimePlayer.InvokesCallbackForEverySample` | Callback count == sample count |
| `RealTimePlayer.StatsReflectSampleCount` | `stats.sample_count` == N |
