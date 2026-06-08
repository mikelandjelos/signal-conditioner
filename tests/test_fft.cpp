#include "fft.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ── Buffer fill / ready flag ──────────────────────────────────────────────────

TEST(FFTProfiler, NotReadyUntilFull) {
  FFTProfiler<16> p(200.0);
  for (int i = 0; i < 15; ++i) {
    p.push(0.0);
    EXPECT_FALSE(p.ready());
  }
  p.push(0.0);
  EXPECT_TRUE(p.ready());
}

TEST(FFTProfiler, ReadyFlagResetAfterQuery) {
  FFTProfiler<16> p(200.0);
  for (int i = 0; i < 16; ++i) {
    p.push(0.0);
  }
  ASSERT_TRUE(p.ready());
  p.dominant_frequencies(1);
  EXPECT_FALSE(p.ready());
}

// ── Silence → spectrum is flat / low ─────────────────────────────────────────

TEST(FFTProfiler, SilenceProducesLowMagnitude) {
  FFTProfiler<64> p(200.0);
  for (int i = 0; i < 64; ++i) {
    p.push(0.0);
  }
  ASSERT_TRUE(p.ready());
  const auto& spec = p.spectrum_db();
  for (std::size_t k = 0; k < spec.size(); ++k) {
    // All bins must be at or below 0 dB for a zero input
    EXPECT_LE(spec[k], 0.0) << "bin " << k << " has unexpected energy";
  }
}

// ── Dominant frequency detection ──────────────────────────────────────────────
// Feed a pure sine at Fs/4 (= bin N/4). The top-1 dominant frequency
// must be close to that frequency.

TEST(FFTProfiler, DISABLED_DetectsDominantFrequency) {
  constexpr std::size_t N = 256;
  const double rate_hz = 200.0;
  const double test_freq = rate_hz / 4.0;  // 50 Hz → bin N/4

  FFTProfiler<N> p(rate_hz);
  for (std::size_t i = 0; i < N; ++i) {
    p.push(std::sin(2.0 * M_PI * test_freq / rate_hz * static_cast<double>(i)));
  }
  ASSERT_TRUE(p.ready());

  auto peaks = p.dominant_frequencies(1);
  ASSERT_FALSE(peaks.empty());

  const double bin_width = rate_hz / static_cast<double>(N);
  EXPECT_NEAR(peaks[0].first, test_freq, bin_width * 2)
      << "Expected peak near " << test_freq << " Hz";
}
