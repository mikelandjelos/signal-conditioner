#include "fft.hpp"
#include "filter.hpp"
#include "ingestion.hpp"
#include "player.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

static void usage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " --dataset <path/to/imu0/data.csv>" << " [--cutoff <hz>]"
            << " [--rate <hz>]" << " [--output <path/to/output.csv>]\n"
            << "\n"
            << "  --dataset   Path to EuRoC imu0/data.csv (required)\n"
            << "  --cutoff    Filter cutoff frequency in Hz (default: 40)\n"
            << "  --rate      Playback rate in Hz (default: 200)\n"
            << "  --output    CSV file for raw+filtered output (default: output.csv)\n";
}

int main(int argc, char* argv[]) {
  std::string dataset_path;
  double cutoff_hz = 40.0;
  double rate_hz = 200.0;
  std::string output = "output.csv";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "--dataset" || arg == "-d") && i + 1 < argc) {
      dataset_path = argv[++i];
    } else if ((arg == "--cutoff" || arg == "-c") && i + 1 < argc) {
      cutoff_hz = std::stod(argv[++i]);
    } else if ((arg == "--rate" || arg == "-r") && i + 1 < argc) {
      rate_hz = std::stod(argv[++i]);
    } else if ((arg == "--output" || arg == "-o") && i + 1 < argc) {
      output = argv[++i];
    } else {
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  if (dataset_path.empty()) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  LogReader reader(dataset_path);
  if (!reader.is_open()) {
    std::cerr << "Error: cannot open " << dataset_path << "\n";
    return EXIT_FAILURE;
  }

  // TODO: open output CSV, write header

  ButterworthFilter filter(cutoff_hz, rate_hz);
  FFTProfiler<1024> profiler(rate_hz);

  RealTimePlayer player(rate_hz);
  player.run(reader, [&](const ImuSample& s, const TimingStats& stats) {
    (void)stats;
    // TODO: push accel z (or chosen axis) through FFT profiler
    // TODO: filter each axis and write raw+filtered row to CSV
    (void)s;
    (void)filter;
    (void)profiler;
  });

  const auto& ts = player.stats();
  std::cout << "Timing — mean: " << ts.mean_ns / 1e6 << " ms  " << "stddev: " << ts.stddev_ns / 1e6
            << " ms  " << "samples: " << ts.sample_count << "\n";

  return EXIT_SUCCESS;
}
