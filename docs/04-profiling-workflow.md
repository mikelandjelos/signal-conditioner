# Profiling Workflow

Two separate concerns: **memory correctness** (no leaks, no invalid accesses) and **timing performance** (loop jitter, throughput). Both are acceptance criteria.

---

## Memory profiling

### AddressSanitizer + UBSan (fast, always available)

Built into the compiler. Zero extra tooling required.

```bash
# Build once
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-asan --parallel

# Run the binary — any error aborts with a stack trace
./build-asan/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null

# Run the test suite under ASan
cd build-asan && ctest --output-on-failure
```

ASan detects: heap-use-after-free, stack buffer overflow, use-after-return, double-free.
UBSan detects: signed integer overflow, null pointer dereference, misaligned access.

### Valgrind / Memcheck (thorough, slower)

Valgrind runs the binary under a binary instrumentation VM — slower (~10–50×) but catches errors ASan misses (uninitialised reads, some race conditions). Verified working with Valgrind 3.22.0.

Install:

```bash
sudo apt install valgrind
```

Build and run:

```bash
# Plain Debug — do NOT use the ASan build; they conflict with Valgrind
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel

# Check the main binary
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --error-exitcode=1 \
  ./build-debug/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null

# Check the test suite
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --error-exitcode=1 \
  ./build-debug/tests/virt_sig_cond_tests
```

Expected for both: `ERROR SUMMARY: 0 errors from 0 contexts` and `All heap blocks were freed`.

Note: the main binary exits with code 1 when run without `--dataset`; this is the binary's own exit code, not a Valgrind error. Always pass a dataset path for a meaningful run.

The acceptance criterion is: **zero dynamic allocations in the steady-state loop**. To confirm this, run Valgrind with `--tool=massif` and inspect the allocation timeline (see below).

### Valgrind Massif (heap allocation timeline)

```bash
valgrind --tool=massif --pages-as-heap=no \
  ./build-debug/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null

ms_print massif.out.<pid> | head -60
```

The heap graph must be flat after startup (no growing staircase during the replay loop).

---

## Timing / performance profiling

### Built-in timing report

The application prints a timing summary on exit:

```
Timing — mean: 5.001 ms  stddev: 0.023 ms  samples: 10000
```

The acceptance criterion is **stddev ≤ 0.5 ms** over 10,000 consecutive samples at 200 Hz.

### perf (Linux)

```bash
# Record a 10-second run
perf record -g ./build/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null

# Annotated call graph
perf report --stdio | head -60
```

Focus on the hot path: `LogReader::next()` → `ButterworthFilter::process()` → output write. Any function consuming >5% of cycles in the steady-state loop is a candidate for optimisation.

### gprof (alternative)

```bash
cmake -B build-prof -DCMAKE_BUILD_TYPE=Release
cmake --build build-prof -- CXXFLAGS="-pg" LDFLAGS="-pg"
./build-prof/virt_sig_cond --dataset data/imu0/data.csv --output /dev/null
gprof build-prof/virt_sig_cond gmon.out | head -40
```

---

## Future: automated profiling report (not yet implemented)

A planned `scripts/profile_report.py` would:

1. Run the application and capture the timing CSV.
2. Run `ms_print` on Massif output and parse the peak heap snapshot.
3. Run the test suite and parse ctest XML results.
4. Emit a Markdown report (`profiling_report.md`) with:
   - Timing histogram (mean, stddev, min, max, P99) per run.
   - Heap allocation timeline figure.
   - Pass/fail table for all acceptance criteria.
   - A "before/after" section comparing Direct-Form I vs. Direct-Form II numerical stability (stretch goal).

Inputs the script should accept:

```
python3 scripts/profile_report.py \
  --timing-csv   output.csv \
  --massif-out   massif.out.<pid> \
  --ctest-xml    build/Testing/*/Test.xml \
  --output       profiling_report.md
```

The CMake target `profile_report` (not yet added) would orchestrate all three runs and invoke the script automatically.
