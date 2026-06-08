add_executable(virt_sig_cond_tests
  test_filter.cpp
  test_fft.cpp
  test_ingestion.cpp
  test_player.cpp
)

target_link_libraries(virt_sig_cond_tests
  PRIVATE
    virt_sig_cond_lib
    GTest::gtest_main
)

target_compile_options(virt_sig_cond_tests PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
  -g -O0
)

include(GoogleTest)
gtest_discover_tests(virt_sig_cond_tests)
