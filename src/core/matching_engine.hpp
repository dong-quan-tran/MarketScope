#pragma once

#include <vector>

#include "order_book.hpp"
#include "trade.hpp"

namespace bookforge {

struct MatchResult {
    std::vector<Trade> trades;
    std::uint32_t remaining_quantity {0};
};

class MatchingEngine {
public:
    MatchingEngine() = default;

    OrderBook& Book() { return book_; }
    const OrderBook& Book() const { return book_; }

    MatchResult MatchLimitOrder(const Order& order);

private:
    OrderBook book_;
};

}  // namespace bookforge