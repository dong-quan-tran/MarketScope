#include "matching_engine.hpp"

namespace bookforge {

namespace {

bool IsCrossingLimit(const Order& order,
                     const std::optional<double>& best_bid,
                     const std::optional<double>& best_ask) {
    if (order.side == Side::Buy) {
        if (!best_ask.has_value()) {
            return false;
        }
        return order.price >= *best_ask;
    }

    if (!best_bid.has_value()) {
        return false;
    }
    return order.price <= *best_bid;
}

}  // namespace

MatchResult MatchingEngine::MatchLimitOrder(const Order& incoming) {
    MatchResult result{};
    std::uint32_t remaining = incoming.quantity;

    auto best_bid = book_.GetBestBid();
    auto best_ask = book_.GetBestAsk();

    if (!IsCrossingLimit(incoming, best_bid, best_ask)) {
        // Not crossing: just rest the order in the book.
        if (book_.AddOrder(incoming)) {
            result.remaining_quantity = 0;
        } else {
            // Duplicate id: nothing changed, all quantity "remaining".
            result.remaining_quantity = incoming.quantity;
        }
        return result;
    }

    // Crossing: we only match against the best price on the opposite side.
    if (incoming.side == Side::Buy) {
        const double match_price = *best_ask;

        auto level_volume = book_.GetLevelVolume(Side::Sell, match_price);
        if (!level_volume.has_value()) {
            // Defensive: no volume at best_ask anymore, just rest the order.
            if (book_.AddOrder(incoming)) {
                result.remaining_quantity = 0;
            } else {
                result.remaining_quantity = incoming.quantity;
            }
            return result;
        }

        const std::uint32_t executed_qty =
            remaining <= *level_volume ? remaining : *level_volume;

        // For now, we collapse the fills at this price into one Trade record,
        // even though multiple maker orders may be consumed under the hood.
        Trade trade{
            incoming.id,
            0,
            Side::Buy,
            match_price,
            executed_qty
        };
        result.trades.push_back(trade);

        // Execute in the book.
        book_.ExecuteTopOrder(Side::Sell, match_price, executed_qty);
        remaining -= executed_qty;
    } else {
        const double match_price = *best_bid;

        auto level_volume = book_.GetLevelVolume(Side::Buy, match_price);
        if (!level_volume.has_value()) {
            if (book_.AddOrder(incoming)) {
                result.remaining_quantity = 0;
            } else {
                result.remaining_quantity = incoming.quantity;
            }
            return result;
        }

        const std::uint32_t executed_qty =
            remaining <= *level_volume ? remaining : *level_volume;

        Trade trade{
            incoming.id,
            0,
            Side::Sell,
            match_price,
            executed_qty
        };
        result.trades.push_back(trade);

        book_.ExecuteTopOrder(Side::Buy, match_price, executed_qty);
        remaining -= executed_qty;
    }

    // For this first version, if there is remaining quantity, we rest it.
    if (remaining > 0) {
        Order resting = incoming;
        resting.quantity = remaining;
        // Reuse same id for now; a future version might assign new ids for residuals.
        if (!book_.AddOrder(resting)) {
            // If insert fails (duplicate id), just report remaining.
        }
    }

    result.remaining_quantity = 0;
    return result;
}

}  // namespace bookforge