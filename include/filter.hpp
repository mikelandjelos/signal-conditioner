#pragma once

#include <array>

// 2nd-order Butterworth low-pass filter via the pre-warped bilinear transform.
// Implements Direct-Form I: y[n] = (b0*x[n] + b1*x[n-1] + b2*x[n-2]
//                                 - a1*y[n-1] - a2*y[n-2]) / a0
//
// Coefficients are derived analytically from the continuous prototype:
//   H(s) = Ωc² / (s² + √2·Ωc·s + Ωc²)
// mapped to digital domain with pre-warped cutoff:
//   Ωc = (2/T) · tan(ωc·T / 2)
//
// Zero steady-state heap allocation — all state is fixed-size on the stack.
//
// Usage:
//   ButterworthFilter f(40.0, 200.0);   // 40 Hz cutoff, 200 Hz sample rate
//   double y = f.process(x);
class ButterworthFilter {
 public:
  ButterworthFilter(double cutoff_hz, double sample_rate_hz);

  // Process one sample. Call once per loop tick.
  double process(double x) noexcept;

  void reset() noexcept;

  double cutoff_hz() const { return cutoff_hz_; }
  double sample_rate_hz() const { return sample_rate_hz_; }

 private:
  void compute_coefficients();

  double cutoff_hz_;
  double sample_rate_hz_;

  // Normalised coefficients (a0 folded in at construction time)
  double b0_, b1_, b2_;
  double a1_, a2_;  // a0 is 1.0 after normalisation

  // Direct-Form I delay lines
  std::array<double, 2> x_hist_{};  // x[n-1], x[n-2]
  std::array<double, 2> y_hist_{};  // y[n-1], y[n-2]
};
