#pragma once

#include <vector>

#include "ExternalOrderEvent.hpp"
#include "ReplayConfig.hpp"
#include "HyperliquidMatchingEngineAdapter.hpp"

namespace bookforge {

class ReplayRunner {
public:
    explicit ReplayRunner(const ReplayConfig& config)
        : config_(config) {}

    // Runs replay against a matching-engine adapter using a pre-loaded event set.
    // Returns true if replay completed successfully under the configured strictness rules.
    bool Run(HyperliquidMatchingEngineAdapter& adapter,
             const std::vector<ExternalOrderEvent>& events) const;

private:
    ReplayConfig config_;
};

}  // namespace bookforge