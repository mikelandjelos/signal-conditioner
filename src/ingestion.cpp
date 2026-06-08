#include "ingestion.hpp"

#include <sstream>
#include <string>

LogReader::LogReader(const std::string& path, Format fmt) : file_(path), fmt_(fmt) {}

std::optional<ImuSample> LogReader::next() {
  // TODO: implement streaming CSV parse for EuRoC / KITTI formats.
  // EuRoC row: timestamp_ns, gx, gy, gz, ax, ay, az
  // KITTI row: differs — handle via fmt_ branch.
  (void)fmt_;
  (void)header_skipped_;
  return std::nullopt;
}
