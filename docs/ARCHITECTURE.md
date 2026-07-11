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
- OFI levels 1�5
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

# Bookforge Architecture

## Week 1 scope

Implemented the core C++ order book:
- `Order` value type
- `PriceLevel` container
- `OrderBook` with bid and ask maps
- Add, cancel, and execute operations
- Best bid/ask, mid-price, spread, and depth queries
- Google Test coverage for correctness and FIFO behavior
- Latency benchmark for core operations

## Core structures

- `OrderBook` stores bids in `std::map<double, PriceLevel, std::greater<>>`.
- `OrderBook` stores asks in `std::map<double, PriceLevel, std::less<>>`.
- `PriceLevel` stores FIFO orders at one price.
- `order_index_` maps `order_id` to order location metadata for direct cancellation lookup.

## Current design notes

- Price levels are sorted by price using ordered maps.
- Orders at the same price follow FIFO behavior.
- Cancellation is keyed by order id through an index maintained by the book.
- This version prioritizes correctness and a clean API before deeper optimization.

## Benchmark method

Benchmark executable:
- `benchmark_order_book`

Timing method:
- `std::chrono::steady_clock`

Operations measured:
- `AddOrder`
- `CancelOrder`
- `ExecuteTopOrder`

Configuration:
- Build type: `Debug` or `Release`
- Iterations: 100000

## Benchmark results

_To be filled after running benchmark executable._

Example template:

- AddOrder: `... ns/op`
- CancelOrder: `... ns/op`
- ExecuteTopOrder: `... ns/op`

## Next steps

- Reject duplicate order ids on insert.
- Add stricter benchmark methodology in Release mode.
- Expand architecture notes as matching logic and feeds are added.
