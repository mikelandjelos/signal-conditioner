#include "ingestion.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

namespace {

// Write a minimal EuRoC-format CSV to a temp file and return its path.
std::string write_euroc_csv(const std::vector<ImuSample>& samples) {
  char tmpl[] = "/tmp/imu_test_XXXXXX";
  int fd = mkstemp(tmpl);
  close(fd);
  std::string path(tmpl);
  std::ofstream f(path);
  f << "#timestamp [ns],w_RS_S_x [rad s^-1],w_RS_S_y [rad s^-1],w_RS_S_z [rad s^-1],"
       "a_RS_S_x [m s^-2],a_RS_S_y [m s^-2],a_RS_S_z [m s^-2]\n";
  for (const auto& s : samples) {
    f << s.timestamp_ns << "," << s.gyro[0] << "," << s.gyro[1] << "," << s.gyro[2] << ","
      << s.accel[0] << "," << s.accel[1] << "," << s.accel[2] << "\n";
  }
  return path;
}

}  // namespace

// ── File open ────────────────────────────────────────────────────────────────

TEST(LogReader, OpensValidFile) {
  auto path = write_euroc_csv({});
  LogReader reader(path);
  EXPECT_TRUE(reader.is_open());
  std::remove(path.c_str());
}

TEST(LogReader, FailsOnMissingFile) {
  LogReader reader("/does/not/exist.csv");
  EXPECT_FALSE(reader.is_open());
}

// ── Parsing ───────────────────────────────────────────────────────────────────

TEST(LogReader, ReturnsNulloptOnEmptyFile) {
  auto path = write_euroc_csv({});
  LogReader reader(path);
  EXPECT_EQ(reader.next(), std::nullopt);
  std::remove(path.c_str());
}

TEST(LogReader, DISABLED_ParsesSingleRow) {
  ImuSample expected{};
  expected.timestamp_ns = 1403636580838555648ULL;
  expected.gyro = {0.1, -0.2, 0.3};
  expected.accel = {9.8, 0.05, -0.1};

  auto path = write_euroc_csv({expected});
  LogReader reader(path);

  auto sample = reader.next();
  ASSERT_TRUE(sample.has_value());
  EXPECT_EQ(sample->timestamp_ns, expected.timestamp_ns);
  EXPECT_NEAR(sample->gyro[0], expected.gyro[0], 1e-9);
  EXPECT_NEAR(sample->gyro[1], expected.gyro[1], 1e-9);
  EXPECT_NEAR(sample->gyro[2], expected.gyro[2], 1e-9);
  EXPECT_NEAR(sample->accel[0], expected.accel[0], 1e-9);
  EXPECT_NEAR(sample->accel[1], expected.accel[1], 1e-9);
  EXPECT_NEAR(sample->accel[2], expected.accel[2], 1e-9);

  // EOF after one row
  EXPECT_EQ(reader.next(), std::nullopt);
  std::remove(path.c_str());
}

TEST(LogReader, DISABLED_ParsesMultipleRows) {
  std::vector<ImuSample> samples(5);
  for (int i = 0; i < 5; ++i) {
    samples[i].timestamp_ns = static_cast<uint64_t>(i) * 5'000'000ULL;
    samples[i].gyro = {static_cast<double>(i), 0.0, 0.0};
    samples[i].accel = {0.0, 0.0, 9.81};
  }

  auto path = write_euroc_csv(samples);
  LogReader reader(path);

  for (int i = 0; i < 5; ++i) {
    auto s = reader.next();
    ASSERT_TRUE(s.has_value()) << "missing sample " << i;
    EXPECT_EQ(s->timestamp_ns, samples[i].timestamp_ns);
    EXPECT_NEAR(s->gyro[0], samples[i].gyro[0], 1e-9);
  }
  EXPECT_EQ(reader.next(), std::nullopt);
  std::remove(path.c_str());
}
