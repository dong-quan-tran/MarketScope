#include "replay/ReplayRunner.hpp"

#include <iostream>

namespace bookforge {

bool ReplayRunner::Run(HyperliquidMatchingEngineAdapter& adapter,
                       const std::vector<ExternalOrderEvent>& events) const {
    const std::uint64_t total = static_cast<std::uint64_t>(events.size());

    const std::uint64_t start = std::min(config_.start_offset, total);
    std::uint64_t limit = total;
    if (config_.max_events > 0) {
        limit = std::min(total, start + config_.max_events);
    }

    if (config_.log_summary) {
        std::cout << "Replaying events from " << start
                  << " to " << limit << " (total available: " << total << ")\n";
    }

    // Deterministic ordering guarantee:
    // We process events strictly in the order they appear in the input vector,
    // from index `start` to `limit - 1`, with no reordering or parallelism.
    for (std::uint64_t i = start; i < limit; ++i) {
        const auto& ev = events[static_cast<std::size_t>(i)];

        // At this level we assume the CSV reader has already validated rows.
        // Errors that occur while handling events are surfaced by adapter logic.
        adapter.OnEvent(ev);

        if (config_.log_every_n > 0 && ((i - start + 1) % config_.log_every_n == 0)) {
            const auto& stats = adapter.Stats();
            std::cout << "Progress: " << (i - start + 1)
                      << " events processed; trades=" << stats.generatedTrades
                      << " ignored=" << stats.ignoredEvents << "\n";
        }
    }

    if (config_.log_summary) {
        const auto& stats = adapter.Stats();
        std::cout << "Replay complete.\n"
                  << "Total events processed: " << (limit - start) << "\n"
                  << "New=" << stats.newCount
                  << " Cancel=" << stats.cancelCount
                  << " Fill=" << stats.fillCount
                  << " Reject=" << stats.rejectCount
                  << " Trigger=" << stats.triggerCount
                  << " Other=" << stats.otherCount << "\n"
                  << "Submitted orders=" << stats.submittedOrders
                  << " Ignored events=" << stats.ignoredEvents
                  << " Generated trades=" << stats.generatedTrades << "\n";
    }

    return true;
}

}  // namespace bookforge