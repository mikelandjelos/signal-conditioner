#include "filter.hpp"

#include <cmath>

ButterworthFilter::ButterworthFilter(double cutoff_hz, double sample_rate_hz)
    : cutoff_hz_(cutoff_hz), sample_rate_hz_(sample_rate_hz) {
  compute_coefficients();
  reset();
}

void ButterworthFilter::compute_coefficients() {
  // TODO: derive b0,b1,b2,a1,a2 via pre-warped bilinear transform.
  // Steps:
  //   T  = 1 / sample_rate_hz_
  //   wc = 2 * pi * cutoff_hz_           (digital cutoff, rad/s)
  //   Wc = (2/T) * tan(wc * T / 2)       (pre-warped analog cutoff)
  //   Apply bilinear s = (2/T)(1-z⁻¹)/(1+z⁻¹) to H(s) = Wc²/(s²+√2·Wc·s+Wc²)
  //   Collect numerator and denominator in powers of z⁻¹, normalise by a0.
  b0_ = b1_ = b2_ = 0.0;
  a1_ = a2_ = 0.0;
}

double ButterworthFilter::process(double x) noexcept {
  // TODO: implement Direct-Form I
  // y = b0_*x + b1_*x_hist_[0] + b2_*x_hist_[1]
  //           - a1_*y_hist_[0] - a2_*y_hist_[1];
  // shift histories
  (void)x;
  return 0.0;
}

void ButterworthFilter::reset() noexcept {
  x_hist_.fill(0.0);
  y_hist_.fill(0.0);
}
