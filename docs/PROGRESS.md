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