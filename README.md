# Bookforge

Bookforge is a hybrid **C++ + Python** market microstructure project built to study how a modern limit order book behaves under real order flow.

The core of the project is a low-latency **C++20 limit order book** with support for order insertion, cancellation, execution, and depth queries. On top of that, the project adds a Python research layer for feature extraction, liquidity estimation, machine learning, and a small API/dashboard demo.

The goal is to build a repo that is both:
- technically strong enough for **quant SWE / quant research interviews**
- practical enough to use for **microstructure experiments and short-horizon signal research**

---

## Project goals

- Build a clean and testable **price-time priority order book**
- Replay historical market events from **real exchange-style data**
- Support both **LOBSTER-style message feeds** and **crypto exchange replay pipelines**
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

This project is being built in stages.

Current completed work:
- Initial repository structure
- Project docs and blueprint
- First version of the C++ `OrderBook`
- First version of the C++ `PriceLevel`
- Unit tests for core order book behavior
- Documented core order book invariants and ownership rules
- Initial Hyperliquid order-status data extraction workflow
- Enriched sample event dataset generation
- CSV-based replay reader for Hyperliquid-style enriched events
- Replay configuration and deterministic replay runner
- Hyperliquid replay adapter wired into the real matching engine
- Parser tests for the Hyperliquid CSV reader
- Replay adapter integration tests
- Bounded replay tests for deterministic replay behavior
- Replay regression wiring through CMake

Current in-progress work:
- LOBSTER-style replay support
- Source-specific field mapping documentation
- Refining the historical data model for realistic cancel / fill handling
- Expanding replay regression coverage with stable fixtures

Planned next work:
- More complete event replay abstractions across multiple data sources
- Feature exporter
- pybind11 bridge
- Python ML pipeline
- API and dashboard

---

## Repository layout

```text
Bookforge/
├── src/
│   ├── core/                  # order, price level, order book
│   ├── features/              # feature extraction from book state
│   ├── matching/              # matching engine logic
│   ├── replay/                # historical message replay / adapters / config
│   ├── ExternalOrderEvent.hpp
│   ├── HyperliquidCsvReader.hpp
│   ├── HyperliquidCsvReader.cpp
│   ├── HyperliquidMatchingEngineAdapter.hpp
│   ├── HyperliquidMatchingEngineAdapter.cpp
│   └── hyperliquid_replay_main.cpp
├── bindings/                  # pybind11 bindings
├── python/
│   ├── lob_engine/            # Python wrapper around C++ core
│   ├── ml/                    # feature engineering, liquidity metrics, training
│   ├── api/                   # FastAPI backend
│   └── dashboard/             # React frontend
├── tests/
│   ├── cpp/                   # GoogleTest suites
│   ├── python/                # Pytest suites
│   └── fixtures/              # replay fixture data
├── data/
│   ├── lobster_sample/
│   └── processed/
├── benchmarks/
├── docs/
└── scripts/
```

---

## Core concepts

### Order book
The order book keeps track of:
- **bids**: buy orders, sorted highest price first
- **asks**: sell orders, sorted lowest price first

Each price level maintains FIFO order priority so that older orders at the same price execute first.

### Event replay
The replay layer ingests **external market events** and maps them into internal engine actions.

The current replay path uses:
- `ExternalOrderEvent` as a stable external event model
- `HyperliquidCsvReader` for enriched CSV parsing
- `ReplayConfig` for source and replay settings
- `ReplayRunner` for deterministic replay traversal
- `HyperliquidMatchingEngineAdapter` to bridge external events into engine-facing actions

Current event fields include:
- `ts`
- `limitPx`
- `sz`
- `isAsk`
- `statusId`
- `status`
- `eventType`

Current `eventType` values include:
- `New`
- `Cancel`
- `Fill`
- `Reject`
- `Trigger`
- `Other`

This keeps exchange-specific parsing separate from the core engine and makes future multi-provider replay support easier.

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
5. Replay those events through the deterministic replay runner and matching engine adapter.

Example enriched columns:

```text
ts,limitPx,sz,isAsk,statusId,status,eventType
2025-12-15 11:39:39.722049503,89691.0,0.01672,True,3,perpMarginRejected,Reject
```

Important limitation:
- the current sample is useful for replay experiments and event classification,
- but it does **not yet provide perfect order lifecycle reconstruction** for every internal engine behavior because exact order-ID-based cancel/fill linkage is still being refined.

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

#### Windows
```powershell
cmake -S . -B build
```

#### macOS / Linux
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

The current replay pipeline is centered on a deterministic event stream.

Main components:
- `ExternalOrderEvent.hpp` — external replay event model
- `HyperliquidCsvReader.*` — parser for enriched Hyperliquid CSV samples
- `ReplayConfig.hpp` — replay configuration object
- `ReplayRunner.*` — deterministic replay traversal
- `HyperliquidMatchingEngineAdapter.*` — bridge into the matching engine
- `hyperliquid_replay_main.cpp` — standalone replay entry point

Current replay behavior:
- `New` events can become internal passive order submissions
- `Reject` events can be logged or ignored
- `Cancel` and `Fill` handling is still evolving as external/internal ID linkage becomes more realistic
- `Trigger` and `Other` can remain deferred until the engine model matures

Testing coverage now includes:
- parser tests for malformed and valid CSV rows,
- integration tests for replay adapter behavior,
- bounded replay tests for deterministic ordering and replay limits,
- replay regression registration through CMake.

This adapter-driven design keeps source-specific logic outside the core order book and makes it easier to support multiple historical data providers later.

---

## Development workflow

A good working loop for this repo:

1. Implement a small feature in C++
2. Add or update Google Tests
3. Build and run C++ tests
4. Validate replay behavior on a small historical sample
5. Expose the feature to Python when needed
6. Add Python-side tests
7. Commit small, focused changes

---

## Documentation

Project planning and notes live in `docs/`:

- `docs/BLUEPRINT.md` — master implementation plan
- `docs/ARCHITECTURE.md` — design choices and benchmark notes
- `docs/DATA_GUIDE.md` — **source-specific field mappings and replay data model**
- `docs/INTERVIEW_PREP.md` — talking points and likely interview questions
- `docs/WEEK_BY_WEEK.md` — development progress log
- `docs/PROGRESS.md` — current implementation progress

---

## Roadmap

### Phase 1 — C++ core
- [x] Order type
- [x] Price level
- [x] Order book skeleton
- [x] Basic order book tests
- [x] Core order book invariants documented
- [ ] More edge-case tests
- [ ] Benchmarks

### Phase 2 — Replay + matching
- [x] First replay event model
- [x] Hyperliquid sample extraction workflow
- [x] Enriched sample CSV generation
- [x] First CSV replay reader
- [x] Replay configuration object
- [x] Deterministic replay runner
- [x] Hyperliquid adapter into matching engine
- [x] Replay adapter integration tests
- [x] Parser tests for replay input
- [x] Bounded replay tests
- [ ] LOBSTER-style message replay
- [ ] Source-specific field mapping docs
- [ ] Snapshot serialization

### Phase 3 — Feature extraction
- [ ] OFI
- [ ] Depth imbalance
- [ ] Spread and mid-price tracking
- [ ] pybind11 bridge

### Phase 4 — Research layer
- [ ] Kyle’s Lambda
- [ ] Label generation
- [ ] XGBoost training
- [ ] Walk-forward evaluation
- [ ] SHAP analysis

### Phase 5 — Demo layer
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
- **systems engineering** through a C++ order book core
- **market microstructure intuition**
- **historical replay infrastructure**
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
- The current Hyperliquid replay path is an early but now test-backed prototype and should be treated as a stepping stone toward a more complete event-driven matching workflow.

---

## License

Add a license before publishing publicly. MIT is a common default for portfolio projects.