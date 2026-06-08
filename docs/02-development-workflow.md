# Core Development Workflow

## Build variants

There are three named build variants. Use whichever matches your current task:

| Variant | Directory | Use for |
|---------|-----------|---------|
| Release | `build/` | Normal dev, timing measurements |
| Debug + ASan | `build-asan/` | Catching memory errors and UB |
| Plain Debug | `build-debug/` | Valgrind (ASan and Valgrind conflict — use this, not `build-asan/`) |

### Release build (default)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Debug build with AddressSanitizer + UBSan

```bash
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-asan --parallel
```

ASan will abort with a stack trace on any heap-use-after-free, stack overflow, or undefined behaviour. Run this before every commit.

### Plain Debug build (for Valgrind)

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

## Rebuild after changes

Incremental builds work as normal — just re-run `cmake --build <dir>`. No need to reconfigure unless you add/remove source files or change CMake options.

## Running the main application

```bash
./build/virt_sig_cond \
  --dataset data/imu0/data.csv \
  --cutoff  40 \
  --rate    200 \
  --output  output.csv
```

| Flag | Default | Description |
|------|---------|-------------|
| `--dataset` / `-d` | required | Path to EuRoC `imu0/data.csv` |
| `--cutoff`  / `-c` | `40.0` | Filter cutoff frequency in Hz |
| `--rate`    / `-r` | `200.0` | Replay rate in Hz |
| `--output`  / `-o` | `output.csv` | Raw + filtered output CSV |

On exit the program prints the timing report:

```
Timing — mean: 5.001 ms  stddev: 0.023 ms  samples: 10000
```

The acceptance criterion is stddev ≤ 0.5 ms over 10,000 samples.

## Validating output visually

```bash
python3 scripts/plot.py output.csv --axis accel_z --cutoff 40
```

Produces `output_plot.png` with a time-series panel and a before/after spectrum panel. The `--axis` flag accepts any column name prefix present in `output.csv` (e.g., `accel_x`, `gyro_z`).

## API documentation

```bash
cmake --build build --target docs
# Opens: docs/doxygen/html/index.html
```

Doxygen is an optional dependency — if not installed the `docs` target is skipped and CMake prints a hint. `docs/doxygen/` is gitignored.

## Formatting

Always format before committing. Both the Claude Code hook and the git pre-commit hook (`.git/hooks/pre-commit`) enforce this, but you can run it manually:

```bash
clang-format -i include/*.hpp src/*.cpp tests/*.cpp
```

To check without modifying (what the hook does):

```bash
clang-format --dry-run --Werror include/*.hpp src/*.cpp tests/*.cpp
```

## Module responsibilities

| Module | Header | Implementation |
|--------|--------|----------------|
| Log ingestion | `ingestion.hpp` | `src/ingestion.cpp` |
| Real-time player | `player.hpp` | `src/player.cpp` |
| FFT profiler | `fft.hpp` | header-only (template) |
| Butterworth filter | `filter.hpp` | `src/filter.cpp` |

`FFTProfiler` is a header-only template — all instantiation sizes (e.g., `<1024>`, `<256>`) are compiled wherever the header is included. No explicit instantiation is needed.
