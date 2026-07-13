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

    while (remaining > 0) {
        auto best_bid = book_.GetBestBid();
        auto best_ask = book_.GetBestAsk();

        if (!IsCrossingLimit(incoming, best_bid, best_ask)) {
            break;
        }

        if (incoming.side == Side::Buy) {
            auto maker = book_.PeekBestAskOrder();
            if (!maker.has_value()) {
                break;
            }

            if (incoming.participant_id != 0 &&
                incoming.participant_id == maker->participant_id) {
                if (incoming.stp == SelfTradePrevention::CancelNewest) {
                    result.remaining_quantity = incoming.quantity;
                    return result;
                }

                if (incoming.stp == SelfTradePrevention::CancelOldest) {
                    bool canceled = book_.CancelOrder(maker->id);
                    if (!canceled) {
                        break;
                    }
                    continue;
                }
            }

            const std::uint32_t executed_qty =
                remaining <= maker->quantity ? remaining : maker->quantity;

            result.trades.push_back(Trade{
                incoming.id,
                maker->id,
                Side::Buy,
                maker->price,
                executed_qty
            });

            bool ok = book_.ExecuteTopOrder(Side::Sell, maker->price, executed_qty);
            if (!ok) {
                break;
            }

            remaining -= executed_qty;
        } else {
            auto maker = book_.PeekBestBidOrder();
            if (!maker.has_value()) {
                break;
            }

            if (incoming.participant_id != 0 &&
                incoming.participant_id == maker->participant_id) {
                if (incoming.stp == SelfTradePrevention::CancelNewest) {
                    result.remaining_quantity = incoming.quantity;
                    return result;
                }

                if (incoming.stp == SelfTradePrevention::CancelOldest) {
                    bool canceled = book_.CancelOrder(maker->id);
                    if (!canceled) {
                        break;
                    }
                    continue;
                }
            }

            const std::uint32_t executed_qty =
                remaining <= maker->quantity ? remaining : maker->quantity;

            result.trades.push_back(Trade{
                incoming.id,
                maker->id,
                Side::Sell,
                maker->price,
                executed_qty
            });

            bool ok = book_.ExecuteTopOrder(Side::Buy, maker->price, executed_qty);
            if (!ok) {
                break;
            }

            remaining -= executed_qty;
        }
    }

    if (remaining > 0) {
        Order resting = incoming;
        resting.quantity = remaining;

        if (book_.AddOrder(resting)) {
            result.remaining_quantity = 0;
        } else {
            result.remaining_quantity = remaining;
        }
    } else {
        result.remaining_quantity = 0;
    }

    return result;
}

}  // namespace bookforge