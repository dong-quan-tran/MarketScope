#pragma once

#include <cstdint>

namespace marketscope {

enum class Side : std::uint8_t {
    Buy = 0,
    Sell = 1
};

struct Order {
    std::uint64_t id {};
    Side side {Side::Buy};
    double price {0.0};
    std::uint32_t quantity {0};
    std::uint64_t timestamp {0};
};

}  // namespace marketscope
