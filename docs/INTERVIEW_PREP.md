# Interview Prep — MarketScope

## Core Questions

### Why C++ for the order book?
Because the order book is the hot path. C++ gives explicit control over
memory layout, latency, and data structures.

### Why std::map first?
It gives sorted traversal and a clean correctness-first implementation.
Later optimization can replace it with array/tick-indexed structures.

### What is Order Flow Imbalance?
OFI measures net buying versus selling pressure by tracking depth changes
on the bid and ask sides.

### What is Kyle's Lambda?
A liquidity / market impact estimate based on how much price moves for a
given signed order flow.

### How do you avoid lookahead bias?
Only use information available at time t for features, and generate labels
strictly from future events after the feature row is formed.
