#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#include "order.hpp"

namespace marketscope {

class PriceLevel {
public:
    explicit PriceLevel(double price = 0.0);

    void AddOrder(const Order& order);
    bool RemoveOrder(std::uint64_t order_id);
    std::optional<Order> PopFrontOrder();

    [[nodiscard]] double GetPrice() const;
    [[nodiscard]] std::uint32_t TotalVolume() const;
    [[nodiscard]] bool Empty() const;
    [[nodiscard]] std::size_t OrderCount() const;

private:
    double price_;
    std::deque<Order> orders_;
};

}  // namespace marketscope
