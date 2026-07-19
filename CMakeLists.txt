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
include(GoogleTest)

add_library(bookforge_core
  src/core/price_level.cpp
  src/core/order_book.cpp
  src/core/matching_engine.cpp
  src/snapshot/SnapshotBuilder.cpp
  src/snapshot/SnapshotSerializer.cpp
  src/snapshot/SnapshotComparator.cpp
  src/snapshot/SnapshotDeserializer.cpp
  src/snapshot/SnapshotBinarySerializer.cpp
  src/snapshot/SnapshotBinaryDeserializer.cpp
  src/features/FeatureBuilder.cpp
  src/features/FeatureCsvWriter.cpp
)

target_include_directories(bookforge_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(test_order_book
  tests/cpp/test_order_book.cpp
)

target_link_libraries(test_order_book PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_order_book)

add_executable(test_matching_engine
  tests/cpp/test_matching_engine.cpp
)

target_link_libraries(test_matching_engine PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_matching_engine)

add_executable(test_hyperliquid_matching_engine_adapter
  tests/cpp/test_hyperliquid_matching_engine_adapter.cpp
)

target_link_libraries(test_hyperliquid_matching_engine_adapter PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_hyperliquid_matching_engine_adapter)

add_executable(test_hyperliquid_csv_reader
  tests/cpp/test_hyperliquid_csv_reader.cpp
  src/HyperliquidCsvReader.cpp
)

target_include_directories(test_hyperliquid_csv_reader PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_hyperliquid_csv_reader PRIVATE
  GTest::gtest_main
)

gtest_discover_tests(test_hyperliquid_csv_reader)

add_executable(test_replay_runner
  tests/cpp/test_replay_runner.cpp
  src/replay/ReplayRunner.cpp
)

target_include_directories(test_replay_runner PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_replay_runner PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_replay_runner)

add_executable(test_replay_regression
  tests/cpp/test_replay_regression.cpp
  src/HyperliquidCsvReader.cpp
  src/replay/ReplayRunner.cpp
)

target_link_libraries(test_replay_regression PRIVATE
  bookforge_core
  GTest::gtest_main
)

target_include_directories(test_replay_regression PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(test_replay_regression PRIVATE
  BOOKFORGE_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
)

gtest_discover_tests(test_replay_regression)

add_executable(test_replay_adapter
  tests/cpp/test_replay_adapter.cpp
  src/replay/ReplayRunner.cpp
)

target_include_directories(test_replay_adapter PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(test_replay_adapter PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_replay_adapter)

add_executable(hyperliquid_replay_main
  src/hyperliquid_replay_main.cpp
  src/HyperliquidCsvReader.cpp
  src/replay/ReplayRunner.cpp
)

target_include_directories(hyperliquid_replay_main PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hyperliquid_replay_main PRIVATE
  bookforge_core
)

add_executable(benchmark_order_book
  bench/benchmark_order_book.cpp
)

target_link_libraries(benchmark_order_book PRIVATE
  bookforge_core
)

add_executable(test_matching_engine_integration
  tests/cpp/test_matching_engine_integration.cpp
)

target_link_libraries(test_matching_engine_integration PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_matching_engine_integration)

add_executable(test_snapshot
  tests/cpp/test_snapshot.cpp
)

target_link_libraries(test_snapshot PRIVATE
  bookforge_core
  GTest::gtest_main
)

gtest_discover_tests(test_snapshot)

add_executable(test_feature_builder
  tests/cpp/test_feature_builder.cpp
)

target_link_libraries(test_feature_builder
  PRIVATE
    bookforge_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_feature_builder)

