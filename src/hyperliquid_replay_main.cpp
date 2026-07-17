#include "HyperliquidCsvReader.hpp"

#include <exception>
#include <iostream>
#include <string>

using namespace bookforge;

static const char* to_string(EventType t) {
    switch (t) {
    case EventType::New: return "New";
    case EventType::Cancel: return "Cancel";
    case EventType::Fill: return "Fill";
    case EventType::Reject: return "Reject";
    case EventType::Trigger: return "Trigger";
    case EventType::Other: return "Other";
    }
    return "Other";
}

int main() {
    try {
        HyperliquidCsvReader reader("data/btc_orders_sample_enriched.csv");
        auto events = reader.read_all();

        std::size_t newCount = 0;
        std::size_t cancelCount = 0;
        std::size_t fillCount = 0;
        std::size_t rejectCount = 0;
        std::size_t triggerCount = 0;
        std::size_t otherCount = 0;

        for (const auto& ev : events) {
            switch (ev.eventType) {
            case EventType::New:     ++newCount; break;
            case EventType::Cancel:  ++cancelCount; break;
            case EventType::Fill:    ++fillCount; break;
            case EventType::Reject:  ++rejectCount; break;
            case EventType::Trigger: ++triggerCount; break;
            case EventType::Other:   ++otherCount; break;
            }
        }

        std::cout << "Loaded events: " << events.size() << '\n';
        std::cout << "New: " << newCount << '\n';
        std::cout << "Cancel: " << cancelCount << '\n';
        std::cout << "Fill: " << fillCount << '\n';
        std::cout << "Reject: " << rejectCount << '\n';
        std::cout << "Trigger: " << triggerCount << '\n';
        std::cout << "Other: " << otherCount << '\n';

        if (!events.empty()) {
            std::cout << "First event type: " << to_string(events.front().eventType) << '\n';
            std::cout << "Last event type: " << to_string(events.back().eventType) << '\n';
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Replay error: " << ex.what() << '\n';
        return 1;
    }
}