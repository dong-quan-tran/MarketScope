#include "order_book.hpp"

namespace marketscope {

void OrderBook::AddOrder(const Order& order) {
    if (order.side == Side::Buy) {
        auto it = bids_.find(order.price);
        if (it == bids_.end()) {
            it = bids_.emplace(order.price, PriceLevel(order.price)).first;
        }
        it->second.AddOrder(order);
    } else {
        auto it = asks_.find(order.price);
        if (it == asks_.end()) {
            it = asks_.emplace(order.price, PriceLevel(order.price)).first;
        }
        it->second.AddOrder(order);
    }
}

bool OrderBook::CancelOrder(std::uint64_t order_id, Side side, double price) {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) return false;

        const bool removed = it->second.RemoveOrder(order_id);
        if (it->second.Empty()) {
            bids_.erase(it);
        }
        return removed;
    } else {
        auto it = asks_.find(price);
        if (it == asks_.end()) return false;

        const bool removed = it->second.RemoveOrder(order_id);
        if (it->second.Empty()) {
            asks_.erase(it);
        }
        return removed;
    }
}

std::optional<double> OrderBook::GetBestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<double> OrderBook::GetBestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

std::optional<double> OrderBook::GetMidPrice() const {
    const auto best_bid = GetBestBid();
    const auto best_ask = GetBestAsk();

    if (!best_bid.has_value() || !best_ask.has_value()) {
        return std::nullopt;
    }

    return (*best_bid + *best_ask) / 2.0;
}

std::optional<double> OrderBook::GetSpread() const {
    const auto best_bid = GetBestBid();
    const auto best_ask = GetBestAsk();

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

}  // namespace marketscope
