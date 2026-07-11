#include "order_book.hpp"

namespace bookforge {

void OrderBook::AddOrder(const Order& order) {
    if (order.side == Side::Buy) {
        auto it = bids_.find(order.price);
        if (it == bids_.end()) {
            auto [inserted_it, inserted] = bids_.emplace(order.price, PriceLevel(order.price));
            inserted_it->second.AddOrder(order);
        } else {
            it->second.AddOrder(order);
        }
    } else {
        auto it = asks_.find(order.price);
        if (it == asks_.end()) {
            auto [inserted_it, inserted] = asks_.emplace(order.price, PriceLevel(order.price));
            inserted_it->second.AddOrder(order);
        } else {
            it->second.AddOrder(order);
        }
    }
}

bool OrderBook::CancelOrder(std::uint64_t order_id, Side side, double price) {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) {
            return false;
        }

        bool removed = it->second.RemoveOrder(order_id);
        if (!removed) {
            return false;
        }

        if (it->second.Empty()) {
            bids_.erase(it);
        }

        return true;
    }

    auto it = asks_.find(price);
    if (it == asks_.end()) {
        return false;
    }

    bool removed = it->second.RemoveOrder(order_id);
    if (!removed) {
        return false;
    }

    if (it->second.Empty()) {
        asks_.erase(it);
    }

    return true;
}

bool OrderBook::ExecuteTopOrder(Side side, double price, std::uint32_t quantity) {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) {
            return false;
        }

        bool ok = it->second.ReduceFrontOrder(quantity);
        if (!ok) {
            return false;
        }

        if (it->second.Empty()) {
            bids_.erase(it);
        }

        return true;
    }

    auto it = asks_.find(price);
    if (it == asks_.end()) {
        return false;
    }

    bool ok = it->second.ReduceFrontOrder(quantity);
    if (!ok) {
        return false;
    }

    if (it->second.Empty()) {
        asks_.erase(it);
    }

    return true;
}

std::optional<double> OrderBook::GetBestBid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.begin()->first;
}

std::optional<double> OrderBook::GetBestAsk() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

std::optional<double> OrderBook::GetMidPrice() const {
    auto best_bid = GetBestBid();
    auto best_ask = GetBestAsk();

    if (!best_bid.has_value() || !best_ask.has_value()) {
        return std::nullopt;
    }

    return (*best_bid + *best_ask) / 2.0;
}

std::optional<double> OrderBook::GetSpread() const {
    auto best_bid = GetBestBid();
    auto best_ask = GetBestAsk();

    if (!best_bid.has_value() || !best_ask.has_value()) {
        return std::nullopt;
    }

    return *best_ask - *best_bid;
}

std::size_t OrderBook::BidLevelCount() const {
    return bids_.size();
}

std::size_t OrderBook::AskLevelCount() const {
    return asks_.size();
}

std::optional<std::uint32_t> OrderBook::GetLevelVolume(Side side, double price) const {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) {
            return std::nullopt;
        }
        return it->second.TotalVolume();
    }

    auto it = asks_.find(price);
    if (it == asks_.end()) {
        return std::nullopt;
    }

    return it->second.TotalVolume();
}

std::vector<std::pair<double, std::uint32_t>> OrderBook::GetBidDepth(std::size_t levels) const {
    std::vector<std::pair<double, std::uint32_t>> depth;
    depth.reserve(levels);

    std::size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count >= levels) {
            break;
        }
        depth.emplace_back(price, level.TotalVolume());
        ++count;
    }

    return depth;
}

std::vector<std::pair<double, std::uint32_t>> OrderBook::GetAskDepth(std::size_t levels) const {
    std::vector<std::pair<double, std::uint32_t>> depth;
    depth.reserve(levels);

    std::size_t count = 0;
    for (const auto& [price, level] : asks_) {
        if (count >= levels) {
            break;
        }
        depth.emplace_back(price, level.TotalVolume());
        ++count;
    }

    return depth;
}

}  // namespace bookforge