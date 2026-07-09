# Architecture & Design Decisions

## Order Book Data Structure
- Bids: std::map<double, PriceLevel>
- Asks: std::map<double, PriceLevel>
- Each PriceLevel stores FIFO orders at one price
- Goal: preserve price-time priority

## Why std::map?
- Sorted price levels are required for best bid/ask and depth traversal
- Predictable O(log N) operations
- Clear interview story before optimizing to array-by-tick designs

## pybind11 Bridge
- Expose OrderBook and feature vectors to Python
- Use Python for ML and visualization
- Keep hot-path book logic in C++

## Feature Layer
- OFI levels 1–5
- Weighted OFI
- Spread
- Depth imbalance
- Kyle's Lambda

## Benchmark Table
| Operation | Mean | p99 | Throughput |
|---|---|---|---|
| AddOrder | TBD | TBD | TBD |
| CancelOrder | TBD | TBD | TBD |
| ExecuteOrder | TBD | TBD | TBD |
| Python bridge | TBD | TBD | TBD |
