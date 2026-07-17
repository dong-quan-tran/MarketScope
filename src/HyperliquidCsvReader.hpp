#pragma once

#include "ExternalOrderEvent.hpp"

#include <vector>
#include <string>

namespace bookforge {

class HyperliquidCsvReader {
public:
    explicit HyperliquidCsvReader(std::string path);

    std::vector<ExternalOrderEvent> read_all();

private:
    std::string path_;
    EventType map_event_type(const std::string& statusText) const;
};

} // namespace bookforge