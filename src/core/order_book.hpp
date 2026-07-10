#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>
#include <utility>

#include "order.hpp"
#include "price_level.hpp"

namespace marketscope {

class OrderBook {
public:
    void AddOrder(const Order& order);
    bool CancelOrder(std::uint64_t order_id, Side side, double price);

    [[nodiscard]] std::optional<double> GetBestBid() const;
    [[nodiscard]] std::optional<double> GetBestAsk() const;
    [[nodiscard]] std::optional<double> GetMidPrice() const;
    [[nodiscard]] std::optional<double> GetSpread() const;

    [[nodiscard]] std::size_t BidLevelCount() const;
    [[nodiscard]] std::size_t AskLevelCount() const;

private:
    std::map<double, PriceLevel, std::greater<>> bids_;
    std::map<double, PriceLevel, std::less<>> asks_;
};

}  // namespace marketscope
