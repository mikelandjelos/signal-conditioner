#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <optional>
#include <string>

struct ImuSample {
  uint64_t timestamp_ns;
  std::array<double, 3> gyro;   // rad/s  [x, y, z]
  std::array<double, 3> accel;  // m/s²   [x, y, z]
};

// Streaming line-by-line parser — never loads the whole file into memory.
// Usage:
//   LogReader reader("path/to/imu0/data.csv");
//   while (auto sample = reader.next()) { ... }
class LogReader {
 public:
  enum class Format { EuRoC, KITTI };

  explicit LogReader(const std::string& path, Format fmt = Format::EuRoC);

  // Returns the next sample, or std::nullopt at EOF / parse error.
  // No heap allocation per call.
  std::optional<ImuSample> next();

  bool is_open() const { return file_.is_open(); }

 private:
  std::ifstream file_;
  Format fmt_;
  bool header_skipped_ = false;
};
