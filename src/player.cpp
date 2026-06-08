#include "player.hpp"

#include <cmath>
#include <thread>

RealTimePlayer::RealTimePlayer(double rate_hz) : period_(static_cast<long long>(1e9 / rate_hz)) {}

void RealTimePlayer::run(LogReader& reader, SampleCallback cb) {
  // TODO: implement fixed-rate loop.
  // Pattern:
  //   auto deadline = steady_clock::now();
  //   while (auto sample = reader.next()) {
  //       deadline += period_;
  //       std::this_thread::sleep_until(deadline);
  //       // measure actual interval, update running mean/variance (Welford)
  //       cb(*sample, stats_);
  //   }
  (void)reader;
  (void)cb;
}
