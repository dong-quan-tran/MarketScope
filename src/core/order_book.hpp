#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "order.hpp"
#include "price_level.hpp"

namespace marketscope {

class OrderBook {
public:
    void AddOrder(const Order& order);
    bool CancelOrder(std::uint64_t order_id, Side side, double price);
    bool ExecuteTopOrder(Side side, double price, std::uint32_t quantity);

    [[nodiscard]] std::optional<double> GetBestBid() const;
    [[nodiscard]] std::optional<double> GetBestAsk() const;
    [[nodiscard]] std::optional<double> GetMidPrice() const;
    [[nodiscard]] std::optional<double> GetSpread() const;

    [[nodiscard]] std::size_t BidLevelCount() const;
    [[nodiscard]] std::size_t AskLevelCount() const;

    [[nodiscard]] std::optional<std::uint32_t> GetLevelVolume(Side side, double price) const;
    [[nodiscard]] std::vector<std::pair<double, std::uint32_t>> GetBidDepth(std::size_t levels) const;
    [[nodiscard]] std::vector<std::pair<double, std::uint32_t>> GetAskDepth(std::size_t levels) const;

private:
    std::map<double, PriceLevel, std::greater<>> bids_;
    std::map<double, PriceLevel, std::less<>> asks_;
};

}  // namespace marketscope
