# Virtual Signal Conditioner

Hardware-free, real-time IMU signal conditioning pipeline. Replays a public EuRoC/KITTI dataset at true sensor rate, profiles its noise spectrum with a self-written FFT, and attenuates structural vibration with a hand-derived 2nd-order Butterworth IIR filter — zero steady-state heap allocation, verified under Valgrind.

**Stack:** C++17 · CMake · Linux

## Quick start

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/virt_sig_cond --dataset data/imu0/data.csv --cutoff 40 --output output.csv
```

## Documentation

| Doc | Contents |
|-----|----------|
| [`docs/01-project-setup.md`](docs/01-project-setup.md) | Prerequisites, tooling, repo layout, dataset setup |
| [`docs/02-development-workflow.md`](docs/02-development-workflow.md) | Build variants, running, plotting, formatting |
| [`docs/03-testing-workflow.md`](docs/03-testing-workflow.md) | GTest, ctest, disabled tests, acceptance targets |
| [`docs/04-profiling-workflow.md`](docs/04-profiling-workflow.md) | ASan, Valgrind, perf, future report scaffold |

Full project specification: [`project1.pdf`](./project1.pdf)
