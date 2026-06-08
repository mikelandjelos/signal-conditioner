#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>

// Radix-2 Cooley-Tukey FFT over a sliding buffer with Hann windowing.
// N must be a power of two. Pre-allocated at construction — zero heap in steady state.
//
// Usage:
//   FFTProfiler<1024> profiler(200.0);  // 200 Hz sample rate
//   profiler.push(sample);
//   if (profiler.ready()) {
//       auto peaks = profiler.dominant_frequencies(5);
//   }
template <std::size_t N>
class FFTProfiler {
  static_assert((N & (N - 1)) == 0, "FFT size must be a power of two");

 public:
  explicit FFTProfiler(double sample_rate_hz) : sample_rate_hz_(sample_rate_hz) {}

  // Add one sample to the sliding buffer.
  void push(double value) {
    buffer_[head_] = value;
    head_ = (head_ + 1) % N;
    if (count_ < N) {
      ++count_;
    }
    if (count_ == N) {
      ready_ = true;
    }
  }

  // True when the buffer has collected N samples and a fresh spectrum is available.
  bool ready() const { return ready_; }

  // Runs the windowed FFT and returns the top-k (frequency_hz, magnitude_dB) pairs.
  // Resets the ready flag until N more samples are pushed.
  std::vector<std::pair<double, double>> dominant_frequencies(std::size_t top_k) {
    compute_fft();
    ready_ = false;

    std::vector<std::pair<double, double>> bins;
    bins.reserve(N / 2);
    for (std::size_t k = 0; k < N / 2; ++k) {
      double freq = static_cast<double>(k) * sample_rate_hz_ / static_cast<double>(N);
      bins.emplace_back(freq, spectrum_db_[k]);
    }

    std::partial_sort(bins.begin(),
                      bins.begin() + static_cast<std::ptrdiff_t>(std::min(top_k, bins.size())),
                      bins.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    bins.resize(std::min(top_k, bins.size()));
    return bins;
  }

  // Returns the full magnitude spectrum in dB (N/2 bins, DC to Nyquist).
  const std::array<double, N / 2>& spectrum_db() const { return spectrum_db_; }

 private:
  void apply_hann_window() {
    // TODO: implement
    //   for i in 0..N:
    //     w = 0.5 * (1 - cos(2π·i / (N-1)))
    //     windowed_[i] = buffer_[(head_ + i) % N] * w
    windowed_.fill(0.0);
  }

  void compute_fft() {
    apply_hann_window();

    // TODO: implement in-place Cooley-Tukey radix-2 DIT FFT.
    // 1. Bit-reverse permutation
    // 2. Butterfly stages log2(N) times
    // 3. Compute magnitude spectrum: spectrum_db_[k] = 20*log10(|freq_[k]| / (N/2))
    freq_.fill({0.0, 0.0});
    spectrum_db_.fill(-120.0);
  }

  double sample_rate_hz_;
  std::array<double, N> buffer_{};
  std::array<double, N> windowed_{};
  std::array<std::complex<double>, N> freq_{};
  std::array<double, N / 2> spectrum_db_{};
  std::size_t head_ = 0;
  std::size_t count_ = 0;
  bool ready_ = false;
};
