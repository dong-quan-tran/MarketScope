#include "HyperliquidCsvReader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace bookforge {

HyperliquidCsvReader::HyperliquidCsvReader(std::string path)
    : path_(std::move(path)) {}

EventType HyperliquidCsvReader::map_event_type(const std::string& statusText) const {
    if (statusText == "open") {
        return EventType::New;
    }
    if (statusText == "filled") {
        return EventType::Fill;
    }
    if (statusText == "triggered") {
        return EventType::Trigger;
    }
    if (statusText.find("Rejected") != std::string::npos) {
        return EventType::Reject;
    }
    if (statusText.find("canceled") != std::string::npos ||
        statusText.find("Canceled") != std::string::npos) {
        return EventType::Cancel;
    }
    return EventType::Other;
}

std::vector<ExternalOrderEvent> HyperliquidCsvReader::read_all() {
    std::ifstream in(path_);
    if (!in) {
        throw std::runtime_error("Failed to open CSV: " + path_);
    }

    std::string line;
    // skip header
    if (!std::getline(in, line)) {
        return {};
    }

    std::vector<ExternalOrderEvent> events;
    events.reserve(100000); // just a hint

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string tsStr, priceStr, sizeStr, isAskStr, statusIdStr, statusTextStr, eventTypeStr;

        std::getline(ss, tsStr, ',');
        std::getline(ss, priceStr, ',');
        std::getline(ss, sizeStr, ',');
        std::getline(ss, isAskStr, ',');
        std::getline(ss, statusIdStr, ',');
        std::getline(ss, statusTextStr, ',');
        std::getline(ss, eventTypeStr, ',');

        ExternalOrderEvent ev{};

        // ts: keep as raw string for now, or parse later if needed
        // Here we store as nanoseconds since epoch = 0; you can replace with real parsing later.
        ev.ts = std::chrono::nanoseconds{0};

        ev.price   = std::stod(priceStr);
        ev.size    = std::stod(sizeStr);
        ev.isAsk   = (isAskStr == "True" || isAskStr == "true");
        ev.statusId   = std::stoi(statusIdStr);
        ev.statusText = statusTextStr;
        ev.eventType  = map_event_type(statusTextStr);

        events.push_back(ev);
    }

    return events;
}

} // namespace bookforge