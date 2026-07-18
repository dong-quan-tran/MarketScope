# Architecture

## Overview

Bookforge uses a price-time priority limit order book implemented in C++20 as the core systems layer.

The core is responsible for:
- representing orders and price levels,
- maintaining bid and ask books,
- enforcing price-time priority,
- supporting add / cancel / execute / replace style operations,
- exposing top-of-book and depth queries.

The design goal is correctness first, with a structure that can later support replay, feature extraction, benchmarking, and Python bindings.

---

## Core data structures

### Order

`Order` is the atomic unit stored by the book.

It represents:
- `id`: unique order identifier
- `participant_id`: owner / participant identity
- `side`: buy or sell
- `price`: limit price
- `quantity`: remaining live quantity
- `timestamp`: arrival time used for priority
- `self-trade-prevention`: optional STP policy metadata

An `Order` should always represent the current remaining quantity, not the original submitted quantity, once partial executions begin.

### PriceLevel

A `PriceLevel` groups all live orders resting at one exact price on one side of the book.

Responsibilities:
- preserve FIFO order among resting orders at the same price,
- track aggregate quantity at that price,
- expose front-order execution behavior,
- support removal when the level becomes empty.

Conceptually:
- bid levels compete by price descending,
- ask levels compete by price ascending,
- inside one level, queue order is strictly time ordered.

### OrderBook

`OrderBook` owns the two-sided market state:
- bids
- asks
- order lookup/index structures

It exposes:
- order insertion,
- cancellation by order id,
- quantity reduction,
- replacement,
- top-order execution at a price level,
- best bid / best ask,
- mid-price / spread,
- depth snapshots.

---

## Matching priority rules

Bookforge follows price-time priority, which is the standard matching rule for a modern limit order book.

### Price priority

Execution priority is determined first by price:
- for bids, higher prices have priority over lower prices,
- for asks, lower prices have priority over higher prices.

This means an incoming marketable buy order matches the lowest available ask first, while an incoming marketable sell order matches the highest available bid first.

### Time priority

When multiple resting orders exist at the same price, they are executed in arrival order, first in first out.

Any operation that effectively replaces an order should be treated as a loss of queue priority unless explicitly designed otherwise.

---

## Book invariants

The following invariants should always hold after every mutating operation:

1. **Side ordering is valid**
   - Bid levels are ordered from highest price to lowest price.
   - Ask levels are ordered from lowest price to highest price.

2. **FIFO within a price level**
   - Orders resting at the same price retain insertion order.
   - The oldest live order at that level is executed first.

3. **Single live instance per order id**
   - An order id may appear at most once in the live book.
   - Duplicate insertion must fail without changing state.

4. **Aggregate quantity consistency**
   - The total quantity reported for a price level equals the sum of remaining quantities of all live orders in that level.

5. **Top-of-book consistency**
   - `best_bid` is the highest live bid if any bids exist.
   - `best_ask` is the lowest live ask if any asks exist.

6. **Empty-level cleanup**
   - If the final order at a price level is canceled or fully executed, that level is removed from the side map.

7. **Order lookup consistency**
   - Every live order reachable through the global order index must also exist in exactly one price-level queue.
   - Every order in a price-level queue must be discoverable through the global order index.

8. **No zero-quantity resting orders**
   - A live order in the book must have strictly positive remaining quantity.

These invariants are the core correctness contract for tests, replay logic, and later Python bindings.

---

## Empty book behavior

The book must handle missing liquidity explicitly.

Rules:
- if no bids exist, `best_bid` is unavailable,
- if no asks exist, `best_ask` is unavailable,
- if either side is empty, `mid_price` is unavailable,
- if either side is empty, `spread` is unavailable.

This avoids inventing synthetic prices and keeps analytics behavior explicit.

---

## Ownership and lifetime rules

Order ownership should be simple and explicit.

### Live lifetime

An order is considered live only if:
- it exists in the order lookup/index, and
- it is present in exactly one resting queue at one price level.

### Removal rules

An order stops being live when:
- it is canceled,
- it is fully executed,
- it is replaced by removing the old state and inserting a new resting instance.

### Partial execution rules

If an order is partially executed:
- the same logical order remains live,
- only its remaining quantity changes,
- its queue priority is preserved unless the operation is a replace.

### Replace rules

A replace operation should be treated as cancel-and-reinsert semantics for priority:
- changing price loses priority,
- replacing at the same price also loses priority in the current design,
- the replaced order should no longer occupy its previous queue position.

---

## Derived market state

The book exposes several derived values:

- **Best bid**: highest live bid price
- **Best ask**: lowest live ask price
- **Mid-price**: `(best_bid + best_ask) / 2`
- **Spread**: `best_ask - best_bid`

These values are only defined when both sides of the book are non-empty.

---

## Why this structure

This design is a good fit for Bookforge because it supports:
- deterministic testing,
- realistic price-time priority behavior,
- clean replay integration,
- future feature extraction like spread, imbalance, and order flow metrics.

It is also interview-friendly because the invariants and trade-offs can be explained clearly without hiding behind framework complexity.