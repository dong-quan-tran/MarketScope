#include <exception>
#include <iostream>

#include "HyperliquidCsvReader.hpp"
#include "HyperliquidMatchingEngineAdapter.hpp"
#include "core/matching_engine.hpp"

using namespace bookforge;

int main() {
    try {
        HyperliquidCsvReader reader("data/btc_orders_sample_enriched.csv");
        auto events = reader.read_all();

        MatchingEngine engine;
        HyperliquidMatchingEngineAdapter adapter(engine);

        for (const auto& ev : events) {
            adapter.OnEvent(ev);
        }

        const auto& stats = adapter.Stats();

        std::cout << "Total events: " << stats.totalEvents << '\n';
        std::cout << "New: " << stats.newCount << '\n';
        std::cout << "Cancel: " << stats.cancelCount << '\n';
        std::cout << "Fill: " << stats.fillCount << '\n';
        std::cout << "Reject: " << stats.rejectCount << '\n';
        std::cout << "Trigger: " << stats.triggerCount << '\n';
        std::cout << "Other: " << stats.otherCount << '\n';
        std::cout << "Submitted orders: " << stats.submittedOrders << '\n';
        std::cout << "Ignored events: " << stats.ignoredEvents << '\n';
        std::cout << "Generated trades: " << stats.generatedTrades << '\n';

        const auto bestBid = engine.Book().GetBestBid();
        const auto bestAsk = engine.Book().GetBestAsk();
        const auto mid = engine.Book().GetMidPrice();
        const auto spread = engine.Book().GetSpread();

        std::cout << "Final best bid: "
                  << (bestBid.has_value() ? std::to_string(*bestBid) : std::string("N/A"))
                  << '\n';
        std::cout << "Final best ask: "
                  << (bestAsk.has_value() ? std::to_string(*bestAsk) : std::string("N/A"))
                  << '\n';
        std::cout << "Final mid-price: "
                  << (mid.has_value() ? std::to_string(*mid) : std::string("N/A"))
                  << '\n';
        std::cout << "Final spread: "
                  << (spread.has_value() ? std::to_string(*spread) : std::string("N/A"))
                  << '\n';

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Replay error: " << ex.what() << '\n';
        return 1;
    }
}