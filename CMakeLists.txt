cmake_minimum_required(VERSION 3.20)
project(Bookforge VERSION 0.1.0 LANGUAGES CXX)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: prevent GoogleTest from overriding the parent project's
# compiler/runtime settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(bookforge_core
  src/core/price_level.cpp
  src/core/order_book.cpp
  src/core/matching_engine.cpp
)

target_include_directories(bookforge_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_order_book
  tests/cpp/test_order_book.cpp
)

add_executable(test_matching_engine
  tests/cpp/test_matching_engine.cpp
)

target_link_libraries(test_order_book PRIVATE
  bookforge_core
  GTest::gtest_main
)

target_link_libraries(test_matching_engine PRIVATE
  bookforge_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_order_book)
gtest_discover_tests(test_matching_engine)

add_executable(benchmark_order_book
  bench/benchmark_order_book.cpp
)

target_link_libraries(benchmark_order_book PRIVATE
  bookforge_core
)