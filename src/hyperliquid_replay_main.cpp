#include "HyperliquidCsvReader.hpp"
#include "HyperliquidEngineAdapter.hpp"
#include "ReplayEngineStub.hpp"

#include <exception>
#include <iostream>

using namespace bookforge;

int main() {
    try {
        HyperliquidCsvReader reader("data/btc_orders_sample_enriched.csv");
        auto events = reader.read_all();

        ReplayEngineStub engine;
        HyperliquidEngineAdapter<ReplayEngineStub> adapter(engine);

        for (const auto& ev : events) {
            adapter.on_event(ev);
        }

        const auto& stats = adapter.stats();

        std::cout << "Loaded events: " << events.size() << '\n';
        std::cout << "New: " << stats.newCount << '\n';
        std::cout << "Cancel: " << stats.cancelCount << '\n';
        std::cout << "Fill: " << stats.fillCount << '\n';
        std::cout << "Reject: " << stats.rejectCount << '\n';
        std::cout << "Trigger: " << stats.triggerCount << '\n';
        std::cout << "Other: " << stats.otherCount << '\n';
        std::cout << "Submitted to book: " << stats.submittedToBook << '\n';
        std::cout << "Engine stored orders: " << engine.submitted_orders().size() << '\n';

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Replay error: " << ex.what() << '\n';
        return 1;
    }
}