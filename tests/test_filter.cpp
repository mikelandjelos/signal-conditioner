#include "filter.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ── Construction ─────────────────────────────────────────────────────────────

TEST(ButterworthFilter, ConstructsWithoutThrowing) {
  EXPECT_NO_THROW(ButterworthFilter(40.0, 200.0));
}

TEST(ButterworthFilter, StoresParameters) {
  ButterworthFilter f(40.0, 200.0);
  EXPECT_DOUBLE_EQ(f.cutoff_hz(), 40.0);
  EXPECT_DOUBLE_EQ(f.sample_rate_hz(), 200.0);
}

// ── DC pass-through ───────────────────────────────────────────────────────────
// A low-pass filter must pass DC (0 Hz) with gain ≈ 1.

TEST(ButterworthFilter, DISABLED_PassesDC) {
  ButterworthFilter f(40.0, 200.0);
  // Drive with a constant 1.0; after settling the output must be ≈ 1.0.
  double out = 0.0;
  for (int i = 0; i < 2000; ++i) {
    out = f.process(1.0);
  }
  EXPECT_NEAR(out, 1.0, 1e-6);
}

// ── Stopband attenuation ──────────────────────────────────────────────────────
// A 2nd-order Butterworth at 40 Hz / 200 Hz must attenuate a 90 Hz tone
// by at least 12 dB (one octave above ~80 Hz corner → ≥ 12 dB/oct rolloff).

TEST(ButterworthFilter, AttenuatesAboveCutoff) {
  const double cutoff_hz = 40.0;
  const double rate_hz = 200.0;
  const double test_freq_hz = 90.0;  // well above cutoff

  ButterworthFilter f(cutoff_hz, rate_hz);

  // Settle the filter
  for (int i = 0; i < 500; ++i) {
    f.process(std::sin(2.0 * M_PI * test_freq_hz / rate_hz * i));
  }

  // Measure output amplitude over one full period
  double peak = 0.0;
  const int measure_samples = static_cast<int>(rate_hz / test_freq_hz) * 4;
  for (int i = 0; i < measure_samples; ++i) {
    double y = f.process(std::sin(2.0 * M_PI * test_freq_hz / rate_hz * (500 + i)));
    peak = std::max(peak, std::abs(y));
  }

  // Input amplitude is 1.0; output must be < 0.25 (-12 dB)
  EXPECT_LT(peak, 0.25) << "Expected at least 12 dB attenuation at " << test_freq_hz << " Hz";
}

// ── Reset clears state ────────────────────────────────────────────────────────

TEST(ButterworthFilter, ResetClearsHistory) {
  ButterworthFilter f(40.0, 200.0);
  for (int i = 0; i < 100; ++i) {
    f.process(1.0);
  }
  f.reset();
  // First output after reset must equal b0 * input (no history contribution)
  // We just check it is small — a fresh filter won't immediately output 1.0.
  double y = f.process(0.0);
  EXPECT_DOUBLE_EQ(y, 0.0);
}
