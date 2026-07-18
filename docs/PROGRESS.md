Progress — July 9, 2026
Set up the initial Bookforge project layout and docs blueprint for the C++ order book core and future microstructure features.

Implemented the first version of OrderBook and PriceLevel (adds, cancels, executes top order, best bid/ask, mid-price, spread, depth queries).

Fixed a compile issue in ExecuteTopOrder caused by using a ternary with two different std::map types, and rewrote it using clear if/else branches.

Ran the CMake debug build and confirmed all current Google Tests for the order book are passing.

Committed the core implementation and tests as a single focused commit to keep history clean and understandable.

Progress log: 07/10/2026

Core implementation and refactors
Implemented PriceLevel and OrderBook in src/core/price_level.* and src/core/order_book.* with:

Bid/ask side maps:

bids_: std::map<double, PriceLevel, std::greater<>>

asks_: std::map<double, PriceLevel, std::less<>>

Per-level storage of orders in FIFO order using std::list<Order>.

Introduced an OrderLocation struct and an order-id index:

std::unordered_map<std::uint64_t, OrderLocation> order_index_ for direct lookup by order id.

OrderLocation now holds { side, price, PriceLevel::Iterator order_it } so cancel can erase in O(1) at the list node.

Switched PriceLevel from std::deque to std::list:

PriceLevel::AddOrder now returns an iterator to the inserted Order.

PriceLevel::RemoveOrder erases by iterator, eliminating per-level linear scans for cancellation.

Public API and behavior
OrderBook public API after today:

bool AddOrder(const Order& order);

bool CancelOrder(std::uint64_t order_id);

bool ExecuteTopOrder(Side side, double price, std::uint32_t quantity);

Query methods: GetBestBid, GetBestAsk, GetMidPrice, GetSpread, GetLevelVolume, GetBidDepth, GetAskDepth.

Added a duplicate-id guard in AddOrder:

If order_index_ already contains order.id, AddOrder returns false and does not change the book state.

This keeps the order index and price levels consistent even under accidental repeated ids.

Testing
Expanded tests/cpp/test_order_book.cpp to 22 tests, covering:

Empty book behavior for best bid/ask, mid, spread.

Single and multiple orders, same-level aggregation, and best bid/ask tracking.

Depth queries (GetBidDepth, GetAskDepth) including requests for more levels than exist.

Partial and full execution at a price level, with automatic level removal when empty.

Cancel behavior:

CancelOrderRemovesLevelWhenEmpty

CancelMissingOrderReturnsFalse

CancelOrderByIdRemovesAsk

FullExecutionRemovesOrderFromIndex (ensures ids are removed on full fills).

FIFO semantics at a price level:

ExecuteTopOrderConsumesOldestOrderFirst

ExecuteTopOrderPartiallyConsumesOldestBeforeNextOrder

Edge queries:

MidPriceUnavailableWhenOnlyBidExists

SpreadUnavailableWhenOnlyAskExists

Duplicate-id behavior:

AddDuplicateOrderIdReturnsFalse

DuplicateOrderIdDoesNotChangeBookState (best bid, level count, and volume unchanged).

Benchmarking
Added a simple latency benchmark executable:

Target: benchmark_order_book in bench/benchmark_order_book.cpp.

Uses std::chrono::steady_clock to time repeated operations, which is recommended for interval timing.

Benchmarked operations with iterations = 100000 on Windows Debug build:

AddOrder: total_ns=111230700, avg_ns≈1112 ns/op.

CancelOrder: total_ns=4831089700, avg_ns≈48310 ns/op.

ExecuteTopOrder: total_ns=73795936200, avg_ns≈737959 ns/op.

Wired the benchmark into CMake:

add_executable(benchmark_order_book bench/benchmark_order_book.cpp).

Linked against bookforge_core.

Documentation
Updated docs/ARCHITECTURE.md to capture:

Current Week 1 scope and what’s implemented.

Data structures (std::map per side, std::list per level, std::unordered_map index).

Benchmark methodology (steady_clock, iterations, Debug build, operations measured).

Baseline latency numbers from today’s benchmark run.

Current status vs Week 1 blueprint
Define Order struct: Done.

Implement PriceLevel core: Done, now using std::list and iterators.

Implement OrderBook core: Done, including id index.

Add AddOrder, CancelOrder, ExecuteOrder:

Implemented as AddOrder, CancelOrder, ExecuteTopOrder.

Add GetBestBid, GetBestAsk, GetMidPrice, GetSpread: Done.

Write 20+ Google Test cases: 22 tests passing via CTest.

Add latency benchmark: Done (benchmark_order_book).

Record results in docs/ARCHITECTURE.md: Draft done with current baseline.

# Bookforge Progress Log — 2026-07-12

## Summary

Today’s work focused on expanding the matching engine, tightening order book semantics, and improving test reliability. The main outcomes were multi-level limit matching, maker-aware trade generation, replace/reduce order behavior, self-trade prevention with both `CancelNewest` and `CancelOldest`, and stronger test ergonomics through a shared order factory helper.

## Completed work

### Matching engine

- Added limit-order matching through a dedicated `MatchingEngine` entry point.
- Implemented multi-level matching so an incoming order can walk the book across multiple price levels.
- Added maker-aware trade generation so each trade records both taker and maker order IDs.

### Order book behavior

- Added order reduction semantics.
- Added order replacement semantics.
- Verified that reduction preserves priority while replacement causes the order to lose time priority.
- Added tests for same-price and new-price replacement behavior.

### Self-trade prevention

- Added `SelfTradePrevention::CancelNewest`.
- Updated tests to cover `CancelNewest` behavior for both buy-side and sell-side matching.
- Added `SelfTradePrevention::CancelOldest`.
- Added tests verifying that `CancelOldest` removes the resting self-matching order and allows the incoming order to continue matching or rest.

### Test infrastructure

- Added a `MakeOrder` helper to reduce repetitive aggregate initialization in tests.
- Updated tests to use the current `Order` shape consistently.
- Expanded matching engine coverage with maker-aware and self-trade-prevention scenarios.
- Kept the order book test suite aligned with reduce, replace, FIFO, and depth behavior.

## Commits completed today

- `50f048f` — Add matching engine for limit orders
- `00b79a9` — Add multi-level limit order matching
- `7875f91` — Add order reduction and replace semantics
- `31405d3` — Add maker-aware trade generation
- `2625e2b` — Add self-trade prevention CancelNewest
- `3a9ae00` — Update tests with CancelNewest
- `de4c756` — Add MakeOrder helper
- `36ce2e3` — Add CancelOldest
- `e7b4d23` — Add tests for CancelOldest

## Validation

- Rebuilt the project successfully with CMake.
- Confirmed the test suite passed in full.
- Current status: 51 tests passing, 0 failing.

## Notes

This was a strong correctness-focused session. The matching engine now supports more realistic execution behavior, self-trade prevention covers two useful policies, and the test suite is in a much better position to support future refactors.

# Bookforge Progress Log — 2026-07-13

## Summary

Today’s work focused on improving the order book benchmark file and getting it back into a clean, maintainable state before starting Week 2.

## Completed work

- Updated `bench/benchmark_order_book.cpp` to use the current `Order` shape.
- Added a local `MakeOrder` helper to reduce brittle aggregate initialization in the benchmark file.
- Kept the benchmark setup clearer by separating book population from the timed sections for cancel and execute paths.
- Rebuilt and ran the benchmark successfully.
- Captured baseline timing for core order book operations:
  - `AddOrder`
  - `CancelOrder`
  - `ExecuteTopOrder`

## Result

The benchmark now compiles cleanly and provides a usable baseline for future performance work. With that foundation in place, benchmark improvements are in a good stopping point, and Week 2 can begin with a stable reference point.

## 2026-07-16

### Completed
- Explored the Hyperliquid order-status dataset layout and verified the extracted folder structure.
- Loaded and decoded `statuses.csv`, then mapped raw `statusId` values to readable status names.
- Built a Python enrichment flow to merge raw order events with status labels and derive a simplified `eventType`.
- Generated an enriched sample event CSV for replay experiments.
- Added initial C++ replay-side types:
  - `ExternalOrderEvent`
  - `HyperliquidCsvReader`
  - `HyperliquidMessageReplayer`
- Added a basic replay executable to read the enriched CSV and summarize event counts.
- Added an adapter-oriented replay design:
  - `HyperliquidEngineAdapter`
  - `ReplayEngineStub`
- Updated the main `README.md` to reflect the new Hyperliquid replay workflow and current architecture direction.
- Cleaned up git tracking so large raw datasets stay local and only code/docs are committed.

### Key outputs
- Working enriched sample schema:

```text
ts,limitPx,sz,isAsk,statusId,status,eventType
```

- First external-event replay path from CSV into C++.
- First adapter boundary between exchange-specific replay data and internal engine-facing actions.

### Notes
- The current Hyperliquid sample is good enough for replay experiments and event classification.
- Exact cancel/fill semantics are still limited because the current sample workflow does not yet fully model order-ID-based lifecycle reconstruction.

### Next
- Replace the stub path with the real matching engine interface.
- Define how `New`, `Cancel`, and `Fill` should map into internal engine actions.
- Add replay-focused tests for parsing, event classification, and adapter behavior.

## 2026-07-17 — Replay foundation and testing

### Summary

Today’s work pushed Bookforge’s replay layer from an early prototype into a **deterministic, test-backed subsystem**.

Main outcomes:
- Closed out the Phase 1 testing checklist.
- Added a Hyperliquid replay adapter that drives the real matching engine.
- Introduced replay configuration and deterministic replay runner abstractions.
- Hardened the Hyperliquid CSV reader with malformed-row handling.
- Added parser tests, bounded replay tests, and replay regression coverage.
- Updated the Phase 2 checklist to reflect completed replay infrastructure work.

### Completed work

#### Core documentation and checklist
- Documented order book invariants and ownership rules.
- Marked the Phase 1 testing checklist complete.

#### Hyperliquid replay integration
- Added a Hyperliquid-specific adapter that maps external replay events into matching engine actions.
- Wired the Hyperliquid replay executable into the real matching engine adapter.
- Added integration tests for Hyperliquid replay adapter behavior.
- Registered Hyperliquid replay adapter tests in CMake.

#### Replay infrastructure
- Added `ReplayConfig` to hold replay parameters such as source, file path, symbol, offsets, limits, and logging settings.
- Added `ReplayRunner` to replay events in deterministic input order.
- Wired the replay entry point to use the new config and runner abstractions.

#### CSV reader hardening
- Extended `HyperliquidCsvReader` with strict and non-strict parsing modes.
- Added malformed-row handling with optional logging.
- Kept replay behavior deterministic even when skipping bad rows in non-strict mode.

#### Test coverage
- Added parser tests for valid rows, malformed rows, strict-mode failure, and unknown status mapping.
- Added bounded replay tests for `start_offset` and `max_events`.
- Added replay regression coverage and registered it in CMake.

### Commits

- `941aa4a` — Add Hyperliquid adapter for matching engine replay events
- `2a9cdc7` — Wire Hyperliquid replay main into real matching engine adapter
- `05e908b` — Add replay adapter integration tests for Hyperliquid events
- `e1c7a0b` — Register Hyperliquid replay adapter tests in CMake
- `0d28830` — Document order book invariants and ownership rules
- `0493c6d` — Mark Phase 1 testing checklist complete
- `8448044` — Add replay config and runner interfaces
- `8215d4a` — Add malformed-row handling to Hyperliquid CSV reader
- `07aee51` — Wire replay main to config and deterministic runner
- `d7487a6` — Add parser tests for Hyperliquid CSV reader
- `5a2c8ab` — Register replay runner and CSV reader tests in CMake
- `ba8ab8a` — Update Phase 2 replay checklist
- `e16280b` — Add bounded replay tests for deterministic runner
- `7fea817` — Register replay regression

### Phase status

#### Phase 1 — C++ core
Phase 1 testing is now marked complete.

#### Phase 2 — Replay foundation
Completed today:
- Stable external event model
- Replay interfaces in `src/replay/`
- CSV / flat-file replay reader
- Deterministic event ordering guarantees
- Replay statistics and instrumentation
- Replay logging for debugging
- Malformed-row error handling
- Replay configuration object

Also strengthened:
- Parser tests for malformed / missing data
- Bounded replay testing
- Replay regression coverage

### Remaining Phase 2 work

Still open:
- Support LOBSTER-style message replay
- Document source-specific field mappings
- Finalize fixture-based replay regression tests if more stable fixtures are needed

### Notes

The replay path is now strong enough to support:
- controlled sample replays,
- regression-friendly testing,
- future expansion to additional historical data sources.

The next best task is likely **documenting source-specific field mappings**, followed by **LOBSTER-style replay support**.