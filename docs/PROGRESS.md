Progress — July 9, 2026
Set up the initial Bookforge project layout and docs blueprint for the C++ order book core and future microstructure features.

Implemented the first version of OrderBook and PriceLevel (adds, cancels, executes top order, best bid/ask, mid-price, spread, depth queries).

Fixed a compile issue in ExecuteTopOrder caused by using a ternary with two different std::map types, and rewrote it using clear if/else branches.

Ran the CMake debug build and confirmed all current Google Tests for the order book are passing.

Committed the core implementation and tests as a single focused commit to keep history clean and understandable.