#include "ingestion.hpp"
#include "player.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

// Write N EuRoC rows spaced at 5 ms (200 Hz) to a temp file.
std::string make_temp_log(int n_samples) {
  char tmpl[] = "/tmp/player_test_XXXXXX";
  int fd = mkstemp(tmpl);
  close(fd);
  std::string path(tmpl);
  std::ofstream f(path);
  f << "#timestamp,gx,gy,gz,ax,ay,az\n";
  for (int i = 0; i < n_samples; ++i) {
    uint64_t ts = static_cast<uint64_t>(i) * 5'000'000ULL;
    f << ts << ",0.0,0.0,0.0,0.0,0.0,9.81\n";
  }
  return path;
}

}  // namespace

// ── Callback invocation count ─────────────────────────────────────────────────

TEST(RealTimePlayer, DISABLED_InvokesCallbackForEverySample) {
  constexpr int N = 5;
  // Use a high rate so the test doesn't actually sleep long
  const double fast_rate = 10000.0;

  auto path = make_temp_log(N);
  LogReader reader(path);
  RealTimePlayer player(fast_rate);

  int count = 0;
  player.run(reader, [&](const ImuSample&, const TimingStats&) { ++count; });

  EXPECT_EQ(count, N);
  std::remove(path.c_str());
}

// ── Stats accumulate ──────────────────────────────────────────────────────────

TEST(RealTimePlayer, DISABLED_StatsReflectSampleCount) {
  constexpr int N = 10;
  const double fast_rate = 10000.0;

  auto path = make_temp_log(N);
  LogReader reader(path);
  RealTimePlayer player(fast_rate);

  player.run(reader, [](const ImuSample&, const TimingStats&) {});

  EXPECT_EQ(player.stats().sample_count, static_cast<uint64_t>(N));
  std::remove(path.c_str());
}

// ── Timing precision (smoke test) ─────────────────────────────────────────────
// At a high synthetic rate the per-frame stddev must stay well below the period.
// This is a loose sanity check — the 200 Hz / ±0.5 ms acceptance test
// requires a real dataset and is validated separately.

TEST(RealTimePlayer, IntervalStddevReasonable) {
  constexpr int N = 50;
  const double rate_hz = 5000.0;
  const double period_ns = 1e9 / rate_hz;

  auto path = make_temp_log(N);
  LogReader reader(path);
  RealTimePlayer player(rate_hz);

  player.run(reader, [](const ImuSample&, const TimingStats&) {});

  // stddev must be less than 2× the nominal period
  EXPECT_LT(player.stats().stddev_ns, period_ns * 2.0)
      << "stddev_ns=" << player.stats().stddev_ns << " period_ns=" << period_ns;
  std::remove(path.c_str());
}
