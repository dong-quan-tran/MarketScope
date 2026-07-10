cmake_minimum_required(VERSION 3.20)
project(MarketScope VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(marketscope_core
  src/core/price_level.cpp
  src/core/order_book.cpp
)

add_executable(test_order_book
  tests/cpp/test_order_book.cpp
)

target_include_directories(test_order_book PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(test_order_book
  PRIVATE
  marketscope_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_order_book)
