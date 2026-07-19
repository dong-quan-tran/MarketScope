# Bookforge

Bookforge is a hybrid **C++ + Python** market microstructure project for studying how a modern limit order book behaves under replayed market event flow.

At its core is a low-latency **C++20 matching engine and price-time-priority limit order book** with deterministic replay infrastructure, snapshot export, and regression-tested historical event playback. On top of that, the project is intended to grow a Python research layer for feature extraction, liquidity estimation, machine learning, and a lightweight API/dashboard demo.

The goal is to build a repo that is both:

- technically strong enough for **quant SWE / quant research interviews**
- practical enough for **microstructure experiments and short-horizon signal research**

---

## Project goals

- Build a clean and testable **price-time-priority order book**
- Build a realistic **matching engine** around the book
- Replay historical market events from **real exchange-style data**
- Support both **crypto exchange replay pipelines** and future **LOBSTER-style message feeds**
- Export reproducible **book snapshots** and replay checkpoints
- Export useful microstructure features such as **spread**, **depth imbalance**, and **order flow imbalance**
- Estimate liquidity measures such as **Kyle’s Lambda**
- Train a short-horizon ML model to predict **mid-price direction**
- Expose the system through a simple **FastAPI backend** and **dashboard**

---

## Tech stack

### Core systems
- C++20
- CMake
- GoogleTest

### Python / data tooling
- Python 3.11+
- pandas
- numpy
- pybind11
- scikit-learn
- xgboost
- FastAPI
- pytest

### Visualization / app layer
- React
- Recharts
- Docker

---

## Current status

This project is being built in staged phases.

### Completed work

- Initial repository structure
- Project docs and blueprint
- C++ `OrderBook`
- C++ `PriceLevel`
- C++ matching engine
- Unit tests for core order book behavior
- Matching-engine integration tests
- Documented core order book invariants and ownership rules
- Initial Hyperliquid order-status data extraction workflow
- Enriched sample event dataset generation
- CSV-based replay reader for Hyperliquid-style enriched events
- Replay configuration and deterministic replay runner
- Generic replay adapter interface
- Hyperliquid replay adapter wired into the real matching engine
- Replay adapter tests
- Parser tests for the Hyperliquid CSV reader
- Bounded replay tests for deterministic replay behavior
- Replay regression tests with stable fixtures
- Snapshot schema
- Snapshot builder from live engine state
- CSV snapshot serializer
- CSV snapshot deserializer
- Snapshot comparator
- Snapshot unit tests
- Snapshot CSV round-trip tests
- Snapshot schema documentation

### In-progress work

- LOBSTER-style replay support
- More realistic external/internal cancel and fill linkage
- Expanded replay checkpoint validation workflows
- Feature extraction layer

### Planned next work

- Binary snapshot output, if useful
- Feature exporter
- pybind11 bridge
- Python ML pipeline
- API and dashboard

---

## Repository layout

```text
Bookforge/
├── src/
│   ├── core/                       # order, price level, order book, matching engine
│   ├── replay/                     # replay runner, config, adapter interfaces
│   ├── snapshot/                   # snapshot schema, builder, serializer, deserializer, comparator
│   ├── features/                   # feature extraction from book state
│   ├── ExternalOrderEvent.hpp
│   ├── HyperliquidCsvReader.hpp
│   ├── HyperliquidCsvReader.cpp
│   ├── HyperliquidMatchingEngineAdapter.hpp
│   ├── HyperliquidMatchingEngineAdapter.cpp
│   └── hyperliquid_replay_main.cpp
├── bindings/                       # pybind11 bindings
├── python/
│   ├── lob_engine/                 # Python wrapper around C++ core
│   ├── ml/                         # feature engineering, liquidity metrics, training
│   ├── api/                        # FastAPI backend
│   └── dashboard/                  # React frontend
├── tests/
│   ├── cpp/                        # GoogleTest suites
│   ├── python/                     # Pytest suites
│   └── fixtures/                   # replay fixture data
├── data/
│   ├── lobster_sample/
│   └── processed/
├── bench/
├── docs/
└── scripts/
```

---

## Core concepts

### Order book

The order book keeps track of:

- **bids**: buy orders, sorted highest price first
- **asks**: sell orders, sorted lowest price first

Each price level maintains FIFO priority so that older orders at the same price execute first.

### Matching engine

The matching engine processes incoming orders against the current book using price-time priority.

Current covered behavior includes:

- passive order insertion
- aggressive matching against resting liquidity
- partial fills
- multi-level sweeps
- trade generation
- event-log capture for important engine actions

### Event replay

The replay layer ingests **external market events** and maps them into internal engine actions.

The current replay path uses:

- `ExternalOrderEvent` as a stable external event model
- `ReplayConfig` for replay settings and limits
- `ReplayRunner` for deterministic replay traversal
- a generic replay adapter interface for engine-facing event application
- `HyperliquidCsvReader` for enriched Hyperliquid CSV parsing
- `HyperliquidMatchingEngineAdapter` as the current source-specific adapter

Current `ExternalOrderEvent` fields include:

- `ts`
- `price`
- `size`
- `isAsk`
- `statusId`
- `statusText`
- `eventType`

Current `eventType` values include:

- `New`
- `Cancel`
- `Fill`
- `Reject`
- `Trigger`
- `Other`

This keeps source-specific parsing separate from the core engine and makes future multi-provider replay support easier.

### Snapshots

Book state can now be exported as structured snapshots for reproducibility and replay checkpoint validation.

Current snapshot support includes:

- a `BookSnapshot` schema
- top-N bid/ask depth export
- best bid / ask, spread, and mid-price export
- replay timestamps and counters
- CSV serialization
- CSV deserialization
- snapshot comparison utilities
- round-trip tests for deterministic snapshot read/write behavior

### Market microstructure features

The project will compute features such as:

- best bid / best ask
- spread
- mid-price
- bid depth / ask depth
- order flow imbalance
- depth imbalance
- rolling liquidity measures

### ML layer

The Python layer will use exported order book features to train a short-horizon classifier for mid-price movement over the next few events.

---

## Hyperliquid workflow

A small Hyperliquid research pipeline is now part of the project workflow.

Current approach:

1. Extract raw order-status data from downloaded archives.
2. Map raw `statusId` values through lookup tables.
3. Enrich the sample into a readable CSV with semantic event labels.
4. Parse the enriched CSV into `ExternalOrderEvent` values.
5. Replay those events through the deterministic replay runner and adapter layer.
6. Validate replay outcomes using regression fixtures and, where needed, snapshots.

Example enriched columns:

```text
ts,limitPx,sz,isAsk,statusId,status,eventType
2025-12-15 11:39:39.722049503,89691.0,0.01672,True,3,perpMarginRejected,Reject
```

Important limitation:

- the current sample is useful for replay experiments and event classification,
- but it does **not yet provide perfect order lifecycle reconstruction** for every engine behavior because exact order-ID-based cancel/fill linkage is still being refined.

---

## Getting started

### 1. Clone the repo

```bash
git clone <your-repo-url>
cd Bookforge
```

### 2. Create a Python virtual environment

#### Windows PowerShell
```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
```

#### macOS / Linux
```bash
python3 -m venv .venv
source .venv/bin/activate
```

### 3. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 4. Configure the C++ build

```bash
cmake -S . -B build
```

### 5. Build the project

```bash
cmake --build build --config Debug
```

---

## Running tests

### C++ tests

```bash
ctest --test-dir build -C Debug --output-on-failure
```

### Python tests

```bash
pytest tests/python -q
```

### Run everything

```bash
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
pytest tests/python -q
```

---

## Replay development

The current replay pipeline is centered on deterministic external event playback.

Main components:

- `ExternalOrderEvent.hpp` — external replay event model
- `HyperliquidCsvReader.*` — parser for enriched Hyperliquid CSV samples
- `ReplayConfig.hpp` — replay configuration object
- `ReplayRunner.*` — deterministic replay traversal
- replay adapter interface — source-agnostic event application contract
- `HyperliquidMatchingEngineAdapter.*` — Hyperliquid source adapter into the matching engine
- `hyperliquid_replay_main.cpp` — standalone replay entry point

Current replay behavior:

- `New` events can become internal passive or aggressive submissions depending on book state
- `Reject` events can be counted without mutating book state
- `Cancel` and `Fill` handling is still evolving as external/internal ID linkage becomes more realistic
- `Trigger` and `Other` can remain deferred until the engine model matures

Testing coverage includes:

- parser tests for malformed and valid CSV rows
- replay adapter tests
- bounded replay tests for deterministic ordering and replay limits
- replay regression tests with stable fixtures
- matching-engine integration tests

This adapter-driven design keeps source-specific logic outside the core engine and makes it easier to support multiple historical data providers later.

---

## Snapshot development

The snapshot layer is intended to make replay state exportable and reproducible.

Current components:

- `BookSnapshot.hpp` — snapshot schema
- `SnapshotBuilder.*` — builds snapshots from live engine/book state
- `SnapshotSerializer.*` — CSV writer
- `SnapshotDeserializer.*` — CSV reader
- `SnapshotComparator.*` — structured snapshot comparison

Current coverage includes:

- top-of-book field validation
- top-N depth validation
- comparator mismatch reporting tests
- CSV header validation
- CSV round-trip tests

See `docs/SNAPSHOT_SCHEMA.md` for schema details.

---

## Development workflow

A good working loop for this repo:

1. Implement a small feature in C++
2. Add or update Google Tests
3. Build and run C++ tests
4. Validate replay behavior on a small historical sample
5. Validate snapshots or replay checkpoints when relevant
6. Expose the feature to Python when needed
7. Add Python-side tests
8. Commit small, focused changes

---

## Documentation

Project planning and notes live in `docs/`:

- `docs/BLUEPRINT.md` — master implementation plan
- `docs/ARCHITECTURE.md` — design choices and benchmark notes
- `docs/DATA_GUIDE.md` — source-specific field mappings and replay data model
- `docs/SNAPSHOT_SCHEMA.md` — snapshot schema and CSV layout
- `docs/INTERVIEW_PREP.md` — talking points and likely interview questions
- `docs/WEEK_BY_WEEK.md` — development progress log
- `docs/PROGRESS.md` — current implementation progress

---

## Roadmap

### Phase 1 — Core book
- [x] Order type
- [x] Price level
- [x] Order book skeleton
- [x] Basic order book tests
- [x] Core order book invariants documented
- [ ] More edge-case tests
- [ ] Benchmarks

### Phase 2 — Replay foundation
- [x] First replay event model
- [x] Hyperliquid sample extraction workflow
- [x] Enriched sample CSV generation
- [x] First CSV replay reader
- [x] Replay configuration object
- [x] Deterministic replay runner
- [x] Generic replay adapter interface
- [x] Hyperliquid adapter into matching engine
- [x] Replay adapter tests
- [x] Parser tests for replay input
- [x] Bounded replay tests
- [x] Replay regression tests
- [ ] LOBSTER-style message replay
- [ ] Additional source adapters

### Phase 3 — Matching and state export
- [x] Matching engine integration
- [x] Matching-engine integration tests
- [x] Snapshot schema
- [x] Snapshot builder
- [x] CSV snapshot serializer
- [x] CSV snapshot deserializer
- [x] Snapshot comparator
- [x] Snapshot round-trip tests
- [ ] Binary snapshot output
- [ ] Expanded replay checkpoint validation

### Phase 4 — Feature extraction
- [ ] OFI
- [ ] Depth imbalance
- [ ] Spread and mid-price tracking
- [ ] Feature export pipeline
- [ ] pybind11 bridge

### Phase 5 — Research layer
- [ ] Kyle’s Lambda
- [ ] Label generation
- [ ] XGBoost training
- [ ] Walk-forward evaluation
- [ ] SHAP analysis

### Phase 6 — Demo layer
- [ ] FastAPI backend
- [ ] Dashboard
- [ ] End-to-end benchmarks
- [ ] Docker setup

---

## Why this project exists

Most portfolio projects show either:

- machine learning without systems depth, or
- systems code without research depth

Bookforge is meant to combine both:

- **systems engineering** through a C++ order book and matching core
- **market microstructure intuition**
- **historical replay infrastructure**
- **state export and reproducibility**
- **ML experimentation**
- **clear testing and documentation**

That makes it a strong portfolio piece for roles across:

- quant software engineering
- market data / infrastructure engineering
- quant research engineering
- latency-sensitive backend engineering

---

## Notes

- This repo is educational and research-oriented.
- It is **not** a production trading system.
- Historical data formats and replay logic will evolve as the project grows.
- The current Hyperliquid replay path is still an approximation of full lifecycle behavior, but it is now test-backed and regression-checked.
- Snapshot export currently prioritizes deterministic CSV workflows first; binary output can be added later if needed.

---

## License

Add a license before publishing publicly. MIT is a common default for portfolio projects.