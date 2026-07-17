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
- Initial Hyperliquid order-status data extraction workflow
- Enriched sample event dataset generation
- First version of a CSV-based replay reader
- First version of a replay entry point for external event streams

Current in-progress work:
- Event replay integration with the internal matching engine
- Replay adapter layer from external exchange events into internal engine actions
- Refining the historical data model for realistic cancel / fill handling

Planned next work:
- Real matching engine integration for replayed events
- More robust event replay abstractions
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
│   ├── replay/                # historical message replay / adapters
│   ├── ExternalOrderEvent.hpp
│   ├── HyperliquidCsvReader.hpp
│   ├── HyperliquidCsvReader.cpp
│   ├── HyperliquidMessageReplayer.hpp
│   └── hyperliquid_replay_main.cpp
├── bindings/                  # pybind11 bindings
├── python/
│   ├── lob_engine/            # Python wrapper around C++ core
│   ├── ml/                    # feature engineering, liquidity metrics, training
│   ├── api/                   # FastAPI backend
│   └── dashboard/             # React frontend
├── tests/
│   ├── cpp/                   # GoogleTest suites
│   └── python/                # Pytest suites
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
The replay layer is being designed to ingest **external market events** and map them into internal engine actions. The current prototype uses an enriched Hyperliquid sample CSV with fields like:

- `ts`
- `limitPx`
- `sz`
- `isAsk`
- `statusId`
- `status`
- `eventType`

The current `eventType` abstraction includes:
- `New`
- `Cancel`
- `Fill`
- `Reject`
- `Trigger`
- `Other`

This creates a clean boundary between exchange-specific raw data and the internal engine API.[web:1096][web:1428]

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
4. Replay that CSV from C++ through a source adapter layer.

Example enriched columns:

```text
ts,limitPx,sz,isAsk,statusId,status,eventType
2025-12-15 11:39:39.722049503,89691.0,0.01672,True,3,perpMarginRejected,Reject
```

Important limitation:
- the current sample is useful for replay experiments and event classification,
- but it does **not yet provide perfect order lifecycle reconstruction** for every internal engine behavior because exact order-ID-based cancel/fill linkage is still being refined.[web:1096]

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

The current replay prototype is centered on a CSV-driven event stream.

Main components:
- `ExternalOrderEvent.hpp` — external replay event model
- `HyperliquidCsvReader.*` — parser for enriched Hyperliquid CSV samples
- `HyperliquidMessageReplayer.hpp` — replay orchestration
- `hyperliquid_replay_main.cpp` — standalone replay entry point

Near-term replay design:
- `New` events should become internal passive order submissions
- `Cancel` events should eventually map to cancel logic once external/internal ID handling is added
- `Fill` events should eventually map to fill or aggressive execution handling
- `Reject` events can be logged or ignored
- `Trigger` and `Other` can remain deferred until the engine model matures

This adapter-driven design keeps source-specific logic outside the core order book and makes it easier to support multiple historical data providers later.[web:1428]

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
- `docs/DATA_GUIDE.md` — data format notes
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
- [ ] More edge-case tests
- [ ] Benchmarks

### Phase 2 — Replay + matching
- [x] First replay event model
- [x] Hyperliquid sample extraction workflow
- [x] Enriched sample CSV generation
- [x] First CSV replay reader
- [x] First replay executable
- [ ] Adapter from external replay events into real engine calls
- [ ] Matching engine integration
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
- The current Hyperliquid replay path is an early prototype and should be treated as a stepping stone toward a more complete event-driven matching workflow.[web:1096]

---

## License

Add a license before publishing publicly. MIT is a common default for portfolio projects.