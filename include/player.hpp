#pragma once

#include "ingestion.hpp"

#include <chrono>
#include <cstdint>
#include <functional>

struct TimingStats {
  double mean_ns;
  double stddev_ns;
  uint64_t sample_count;
};

// Replays IMU samples at a fixed rate using sleep_until on a monotonic clock.
// Accumulates per-frame interval measurements for the timing acceptance test.
//
// Usage:
//   RealTimePlayer player(200.0);
//   player.run(reader, [](const ImuSample& s, const TimingStats& stats) { ... });
class RealTimePlayer {
 public:
  using SampleCallback = std::function<void(const ImuSample&, const TimingStats&)>;

  explicit RealTimePlayer(double rate_hz);

  // Drives the loop until the reader is exhausted or the callback returns false.
  // Uses steady_clock + sleep_until — never sleep_for.
  void run(LogReader& reader, SampleCallback cb);

  const TimingStats& stats() const { return stats_; }

 private:
  std::chrono::nanoseconds period_;
  TimingStats stats_{};
};
