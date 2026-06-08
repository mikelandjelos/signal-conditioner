cmake_minimum_required(VERSION 3.16)
project(virt_sig_cond VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type defaults to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(ENABLE_ASAN  "Build with AddressSanitizer" OFF)
option(BUILD_TESTS  "Build unit tests" ON)

# ---------------------------------------------------------------------------
# Library target (shared between main binary and tests)
# ---------------------------------------------------------------------------
add_library(virt_sig_cond_lib STATIC
  src/ingestion.cpp
  src/player.cpp
  src/filter.cpp
)

target_include_directories(virt_sig_cond_lib PUBLIC include)

target_compile_options(virt_sig_cond_lib PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
  $<$<CONFIG:Debug>:-g -O0>
  $<$<CONFIG:Release>:-O2>
)

if(ENABLE_ASAN)
  target_compile_options(virt_sig_cond_lib PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(virt_sig_cond_lib PUBLIC -fsanitize=address,undefined)
endif()

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(virt_sig_cond src/main.cpp)
target_link_libraries(virt_sig_cond PRIVATE virt_sig_cond_lib)
target_compile_options(virt_sig_cond PRIVATE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BUILD_TESTS)
  include(FetchContent)

  # Cache downloads outside the build tree so rm -rf build/ doesn't re-fetch.
  # Override with -DFETCHCONTENT_BASE_DIR=<path> if you prefer a different location.
  if(NOT DEFINED FETCHCONTENT_BASE_DIR)
    set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.cmake-deps" CACHE PATH
        "FetchContent download cache (persists across build-dir wipes)" FORCE)
  endif()

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
  )

  # Prevent GTest from overriding compiler/linker settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  # Don't install GTest alongside this project
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Doxygen
# ---------------------------------------------------------------------------
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation (docs/doxygen/html/index.html)"
    VERBATIM
  )
else()
  message(STATUS "Doxygen not found — 'docs' target unavailable. Install with: sudo apt install doxygen")
endif()
