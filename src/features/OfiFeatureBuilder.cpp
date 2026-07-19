#include "features/OfiFeatureBuilder.hpp"

#include <algorithm>
#include <stdexcept>

namespace bookforge {

void OfiFeatureBuilder::AddOfiFeatures(
    std::vector<FeatureRow>& rows,
    const std::vector<BookSnapshot>& snapshots,
    std::size_t depth_levels_for_ofi) {
    if (rows.size() != snapshots.size()) {
        throw std::runtime_error("rows and snapshots size mismatch in OfiFeatureBuilder");
    }

    if (rows.empty()) {
        return;
    }

    rows[0].ofi_l1 = std::nullopt;
    rows[0].ofi_lN = std::nullopt;

    for (std::size_t i = 1; i < rows.size(); ++i) {
        const auto& prev = snapshots[i - 1];
        const auto& curr = snapshots[i];

        const double l1_ofi = ComputeBestLevelOfi(prev, curr);
        const double lN_ofi =
            ComputeMultiLevelOfi(prev, curr, depth_levels_for_ofi);

        rows[i].ofi_l1 = l1_ofi;
        rows[i].ofi_lN = lN_ofi;
    }
}

double OfiFeatureBuilder::ComputeBestLevelOfi(
    const BookSnapshot& prev,
    const BookSnapshot& curr) {
    const std::optional<DepthLevelSnapshot> prev_bid =
        prev.bids.empty() ? std::optional<DepthLevelSnapshot>{}
                          : std::optional<DepthLevelSnapshot>{prev.bids.front()};
    const std::optional<DepthLevelSnapshot> curr_bid =
        curr.bids.empty() ? std::optional<DepthLevelSnapshot>{}
                          : std::optional<DepthLevelSnapshot>{curr.bids.front()};

    const std::optional<DepthLevelSnapshot> prev_ask =
        prev.asks.empty() ? std::optional<DepthLevelSnapshot>{}
                          : std::optional<DepthLevelSnapshot>{prev.asks.front()};
    const std::optional<DepthLevelSnapshot> curr_ask =
        curr.asks.empty() ? std::optional<DepthLevelSnapshot>{}
                          : std::optional<DepthLevelSnapshot>{curr.asks.front()};

    const double ofi_bid = ComputeSideOfiBest(prev_bid, curr_bid, true);
    const double ofi_ask = ComputeSideOfiBest(prev_ask, curr_ask, false);

    return ofi_bid + ofi_ask;
}

double OfiFeatureBuilder::ComputeMultiLevelOfi(
    const BookSnapshot& prev,
    const BookSnapshot& curr,
    std::size_t depth_levels_for_ofi) {
    const double ofi_bid =
        ComputeSideOfiMulti(prev.bids, curr.bids, depth_levels_for_ofi, true);
    const double ofi_ask =
        ComputeSideOfiMulti(prev.asks, curr.asks, depth_levels_for_ofi, false);

    return ofi_bid + ofi_ask;
}

double OfiFeatureBuilder::ComputeSideOfiBest(
    const std::optional<DepthLevelSnapshot>& prev_level,
    const std::optional<DepthLevelSnapshot>& curr_level,
    bool is_bid_side) {
    if (!prev_level.has_value() && !curr_level.has_value()) {
        return 0.0;
    }

    if (!prev_level.has_value() && curr_level.has_value()) {
        const double v_curr = static_cast<double>(curr_level->quantity);
        if (is_bid_side) {
            return +v_curr;
        } else {
            return -v_curr;
        }
    }

    if (prev_level.has_value() && !curr_level.has_value()) {
        const double v_prev = static_cast<double>(prev_level->quantity);
        if (is_bid_side) {
            return -v_prev;
        } else {
            return +v_prev;
        }
    }

    const double p_prev = prev_level->price;
    const double v_prev = static_cast<double>(prev_level->quantity);
    const double p_curr = curr_level->price;
    const double v_curr = static_cast<double>(curr_level->quantity);

    if (is_bid_side) {
        if (p_curr > p_prev) {
            return +v_curr;
        } else if (p_curr < p_prev) {
            return -v_prev;
        } else {
            return v_curr - v_prev;
        }
    } else {
        if (p_curr < p_prev) {
            return -v_curr;
        } else if (p_curr > p_prev) {
            return +v_prev;
        } else {
            return v_prev - v_curr;
        }
    }
}

double OfiFeatureBuilder::ComputeSideOfiMulti(
    const std::vector<DepthLevelSnapshot>& prev_side,
    const std::vector<DepthLevelSnapshot>& curr_side,
    std::size_t depth_levels,
    bool is_bid_side) {
    const std::size_t levels =
        std::min(depth_levels, std::max(prev_side.size(), curr_side.size()));

    double total = 0.0;
    for (std::size_t i = 0; i < levels; ++i) {
        const std::optional<DepthLevelSnapshot> prev_level =
            (i < prev_side.size())
                ? std::optional<DepthLevelSnapshot>{prev_side[i]}
                : std::optional<DepthLevelSnapshot>{};

        const std::optional<DepthLevelSnapshot> curr_level =
            (i < curr_side.size())
                ? std::optional<DepthLevelSnapshot>{curr_side[i]}
                : std::optional<DepthLevelSnapshot>{};

        total += ComputeSideOfiBest(prev_level, curr_level, is_bid_side);
    }

    return total;
}

}  // namespace bookforge