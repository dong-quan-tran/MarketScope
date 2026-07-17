#include "HyperliquidMessageReplayer.hpp"

#include <iostream>

int main() {
    try {
        bookforge::HyperliquidMessageReplayer replayer{
            "data/btc_orders_sample_enriched.csv"
        };
        replayer.replay();
        std::cout << "Replay completed.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Replay error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}