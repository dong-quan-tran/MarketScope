#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bookforge {

struct FeatureRow {
    std::string symbol;
    std::uint64_t replay_event_index{0};
    std::uint64_t replay_timestamp_ns{0};

    std::optional<double> best_bid;
    std::optional<double> best_ask;
    std::optional<double> spread;
    std::optional<double> mid_price;

    std::optional<double> l1_bid_qty;
    std::optional<double> l1_ask_qty;
    std::optional<double> l1_depth_imbalance;

    std::optional<double> lN_bid_qty_sum;
    std::optional<double> lN_ask_qty_sum;
    std::optional<double> lN_depth_imbalance;

    std::optional<double> ofi_l1;
    std::optional<double> ofi_lN;
};

}  // namespace bookforge