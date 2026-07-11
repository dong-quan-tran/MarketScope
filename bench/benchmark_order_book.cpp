#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "core/order_book.hpp"

using namespace bookforge;

namespace {
using Clock = std::chrono::steady_clock;

void BenchmarkAddOrder(std::size_t iterations) {
    OrderBook book;

    const auto start = Clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        const Side side = (i % 2 == 0) ? Side::Buy : Side::Sell;
        const double price = (side == Side::Buy)
            ? 100.00 - static_cast<double>(i % 50) * 0.01
            : 100.50 + static_cast<double>(i % 50) * 0.01;
        const std::uint32_t quantity = 100 + static_cast<std::uint32_t>(i % 25);
        const std::uint64_t timestamp = static_cast<std::uint64_t>(i + 1);

        book.AddOrder(Order{
            static_cast<std::uint64_t>(i + 1),
            side,
            price,
            quantity,
            timestamp
        });
    }
    const auto end = Clock::now();

    const auto total_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "AddOrder: total_ns=" << total_ns
              << ", avg_ns=" << (total_ns / static_cast<long long>(iterations))
              << '\n';
}

void BenchmarkCancelOrder(std::size_t iterations) {
    OrderBook book;

    for (std::size_t i = 0; i < iterations; ++i) {
        const Side side = (i % 2 == 0) ? Side::Buy : Side::Sell;
        const double price = (side == Side::Buy)
            ? 100.00 - static_cast<double>(i % 50) * 0.01
            : 100.50 + static_cast<double>(i % 50) * 0.01;
        const std::uint32_t quantity = 100;
        const std::uint64_t timestamp = static_cast<std::uint64_t>(i + 1);

        book.AddOrder(Order{
            static_cast<std::uint64_t>(i + 1),
            side,
            price,
            quantity,
            timestamp
        });
    }

    const auto start = Clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        book.CancelOrder(static_cast<std::uint64_t>(i + 1));
    }
    const auto end = Clock::now();

    const auto total_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "CancelOrder: total_ns=" << total_ns
              << ", avg_ns=" << (total_ns / static_cast<long long>(iterations))
              << '\n';
}

void BenchmarkExecuteTopOrder(std::size_t iterations) {
    OrderBook book;

    for (std::size_t i = 0; i < iterations; ++i) {
        book.AddOrder(Order{
            static_cast<std::uint64_t>(i + 1),
            Side::Buy,
            100.00,
            100,
            static_cast<std::uint64_t>(i + 1)
        });
    }

    const auto start = Clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        book.ExecuteTopOrder(Side::Buy, 100.00, 100);
    }
    const auto end = Clock::now();

    const auto total_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "ExecuteTopOrder: total_ns=" << total_ns
              << ", avg_ns=" << (total_ns / static_cast<long long>(iterations))
              << '\n';
}
}  // namespace

int main() {
    constexpr std::size_t kIterations = 100000;

    std::cout << "Bookforge OrderBook benchmark\n";
    std::cout << "iterations=" << kIterations << '\n';

    BenchmarkAddOrder(kIterations);
    BenchmarkCancelOrder(kIterations);
    BenchmarkExecuteTopOrder(kIterations);

    return 0;
}