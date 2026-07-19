# Snapshot schema

## Overview

A snapshot captures a deterministic view of reconstructed book state at a specific replay checkpoint.

Each snapshot is intended to be:

- exportable to disk,
- reproducible across replay runs,
- comparable against expected checkpoint state,
- easy to inspect in plain text form.

The initial Phase 5 format is CSV-first. Binary snapshot output may be added later after the schema stabilizes.

## Snapshot model

A snapshot contains:

- symbol metadata,
- replay checkpoint metadata,
- top-of-book derived fields,
- top-N bid depth,
- top-N ask depth,
- replay/event counters.

## Scalar fields

Each snapshot includes the following scalar fields:

| Field | Type | Description |
|---|---|---|
| `symbol` | string | Logical symbol being replayed. |
| `replay_event_index` | uint64 | 1-based or project-defined replay event position at capture time. |
| `replay_timestamp_ns` | uint64 | Replay timestamp in nanoseconds for the triggering event. |
| `total_events_seen` | uint64 | Number of replay events processed so far. |
| `submitted_orders` | uint64 | Count of accepted/submitted order events seen so far. |
| `rejected_events` | uint64 | Count of rejected events seen so far. |
| `ignored_events` | uint64 | Count of ignored or unsupported events seen so far. |
| `generated_trades` | uint64 | Count of trades generated so far. |
| `best_bid` | optional double | Highest resting bid price, if present. |
| `best_ask` | optional double | Lowest resting ask price, if present. |
| `mid_price` | optional double | Midpoint of best bid and best ask, if both exist. |
| `spread` | optional double | Best ask minus best bid, if both exist. |

## Depth fields

Each snapshot also includes top-N aggregated book depth on both sides.

A depth level contains:

| Field | Type | Description |
|---|---|---|
| `price` | double | Price of the level. |
| `quantity` | uint32 | Aggregated resting quantity at that level. |

Rules:

- Bid levels are sorted from highest price to lowest price.
- Ask levels are sorted from lowest price to highest price.
- Only the top `N` levels per side are exported.
- Missing levels in CSV output are emitted as empty cells.

## CSV layout

The CSV format uses one row per snapshot.

### Scalar prefix

The row begins with:

```text
symbol,replay_event_index,replay_timestamp_ns,total_events_seen,submitted_orders,rejected_events,ignored_events,generated_trades,best_bid,best_ask,mid_price,spread
```

### Bid depth section

For each bid level `i` from `1` to `N`:

```text
bid_px_i,bid_qty_i
```

Example for `N = 3`:

```text
bid_px_1,bid_qty_1,bid_px_2,bid_qty_2,bid_px_3,bid_qty_3
```

### Ask depth section

For each ask level `i` from `1` to `N`:

```text
ask_px_i,ask_qty_i
```

Example for `N = 3`:

```text
ask_px_1,ask_qty_1,ask_px_2,ask_qty_2,ask_px_3,ask_qty_3
```

## Example row shape

For `N = 2`, the full row shape is:

```text
symbol,replay_event_index,replay_timestamp_ns,total_events_seen,submitted_orders,rejected_events,ignored_events,generated_trades,best_bid,best_ask,mid_price,spread,bid_px_1,bid_qty_1,bid_px_2,bid_qty_2,ask_px_1,ask_qty_1,ask_px_2,ask_qty_2
```

## Comparison semantics

Snapshots are compared structurally.

A comparison checks:

- scalar metadata equality,
- best bid / ask / mid / spread equality,
- bid depth size equality,
- ask depth size equality,
- per-level price and quantity equality.

Comparison output should identify the first mismatching field or depth index to support replay checkpoint debugging.

## Replay checkpoint usage

Snapshots are intended to be captured at deterministic replay checkpoints, such as:

- after a fixed event index,
- after a bounded replay range,
- after a known fixture sequence,
- after a trade-producing event.

These checkpoints allow regression tests to validate that reconstructed book state remains stable across code changes.

## Binary snapshot format

Bookforge also supports a versioned binary snapshot format for compact, deterministic snapshot storage.

### File header

Each binary snapshot file begins with:

- `magic[8]`: ASCII bytes `BFSNAP01`
- `version` (`uint32`, little-endian): current value `1`
- `reserved` (`uint32`, little-endian): must be `0`
- `depth_levels` (`uint32`, little-endian): configured top-N depth
- `snapshot_count` (`uint64`, little-endian): number of snapshot records in the file

### Snapshot record layout

Each snapshot record is encoded as:

1. `symbol_len` (`uint32`)
2. `symbol_bytes` (`symbol_len` raw bytes, no null terminator)
3. `replay_event_index` (`uint64`)
4. `replay_timestamp_ns` (`uint64`)
5. `total_events_seen` (`uint64`)
6. `submitted_orders` (`uint64`)
7. `rejected_events` (`uint64`)
8. `ignored_events` (`uint64`)
9. `generated_trades` (`uint64`)
10. `best_bid_present` (`uint8`)
11. `best_bid` (`double`) if present
12. `best_ask_present` (`uint8`)
13. `best_ask` (`double`) if present
14. `mid_price_present` (`uint8`)
15. `mid_price` (`double`) if present
16. `spread_present` (`uint8`)
17. `spread` (`double`) if present
18. `bid_count` (`uint32`)
19. repeated `bid_count` times:
    - `price` (`double`)
    - `quantity` (`uint32`)
20. `ask_count` (`uint32`)
21. repeated `ask_count` times:
    - `price` (`double`)
    - `quantity` (`uint32`)

### Format guarantees

- All integers use fixed-width little-endian encoding.
- Floating-point values are written as IEEE-754 `double` bit patterns.
- Strings are length-prefixed and stored without null terminators.
- The format is versioned so future schema updates can be handled explicitly.
- The binary format is deterministic for the same snapshot content and depth configuration.

## Versioning notes

The Phase 5 schema is the initial snapshot schema.

If fields are added later:

- preserve existing column names where possible,
- append new columns rather than renaming old ones,
- version binary formats explicitly if binary serialization is introduced.