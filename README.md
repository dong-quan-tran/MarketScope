# Bookforge

Bookforge is a hybrid **C++ + Python** market microstructure project for studying how a modern limit order book behaves under replayed market event flow.

At its core is a low-latency **C++20 matching engine and price-time-priority order book** with deterministic replay infrastructure, snapshot export, feature export, and regression-tested historical event playback. On top of that, the project includes a Python research layer for dataset construction, short-horizon machine learning, walk-forward evaluation, feature importance / SHAP analysis, experiment tracking with MLflow, and a lightweight **FastAPI + React dashboard** for inspection and demos.

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
- Train short-horizon ML models to predict **mid-price direction**
- Evaluate models with **chronological holdout** and **walk-forward validation**
- Track experiments and artifacts with **MLflow**
- Expose replay outputs through a simple **FastAPI backend** and **React dashboard**

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
- scipy
- pybind11
- scikit-learn
- xgboost
- shap
- mlflow
- pytest

### API / app layer
- FastAPI
- Pydantic
- Uvicorn
- React
- Vite
- Recharts
- Docker
- Docker Compose

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
- Sample event dataset generation
- CSV-based replay reader for Hyperliquid-style events
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
- Feature export CLI
- Python feature CSV loader and validation utilities
- Python training dataset builder
- pybind11-backed package structure
- First XGBoost baseline training pipeline
- Chronological holdout evaluation
- Walk-forward validation
- Feature importance export
- Optional SHAP analysis
- MLflow experiment tracking
- Pytest coverage for Python wrappers
- FastAPI service for replay inspection
- Replay summary API endpoint
- Feature sample retrieval API endpoint
- React + Vite dashboard for replay feature visualization
- Summary cards and charts for spread, mid-price, depth, and imbalance
- API contract tests
- Docker-based local demo setup

### In-progress work

- LOBSTER-style replay support
- More realistic external/internal cancel and fill linkage
- Label-quality improvements for better class balance
- Liquidity estimation features such as Kyle’s Lambda
- Book snapshot inspection endpoint
- Better dashboard controls for replay slicing and inspection

### Planned next work

- Binary snapshot output, if useful
- Expanded replay checkpoint validation workflows
- Better label diagnostics and confusion-matrix reporting
- More realistic multi-class / imbalanced evaluation
- Richer end-to-end demo and deployment polish

---

## Repository layout

```text
Bookforge/
├── src/
│   ├── core/                       # order, price level, order book, matching engine
│   ├── replay/                     # replay runner, config, adapter interfaces
│   ├── snapshot/                   # snapshot schema, builder, serializer, deserializer, comparator
│   ├── features/                   # feature extraction from book state
│   ├── python/                     # pybind11 C++ binding source
│   │   └── bookforge_py.cpp
│   ├── ExternalOrderEvent.hpp
│   ├── HyperliquidCsvReader.hpp
│   ├── HyperliquidCsvReader.cpp
│   ├── HyperliquidMatchingEngineAdapter.hpp
│   ├── HyperliquidMatchingEngineAdapter.cpp
│   ├── hyperliquid_replay_main.cpp
│   └── feature_export_main.cpp
├── python/
│   ├── bookforge_py/               # Python package: loaders, dataset helpers, wrapper exports
│   ├── ml/                         # training and evaluation scripts
│   └── api/                        # FastAPI backend
├── dashboard/                      # Vite + React dashboard
├── tests/
│   ├── cpp/                        # GoogleTest suites
│   ├── python/                     # Pytest suites
│   └── fixtures/                   # replay fixture data
├── data/
│   ├── lobster_sample/
│   └── processed/
├── docs/
├── build/
├── output/
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
- `HyperliquidCsvReader` for Hyperliquid CSV parsing
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

Book state can be exported as structured snapshots for reproducibility and replay checkpoint validation.

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

The project exports a first feature set including:

- best bid / best ask
- spread
- mid-price
- L1 bid / ask size
- depth imbalance
- order flow imbalance
- rolling feature statistics
- replay metadata for supervised dataset construction

### ML layer

The Python layer supports:

- feature CSV loading and validation
- training dataset construction
- label generation
- chronological holdout evaluation
- walk-forward validation
- XGBoost baseline training
- feature importance export
- optional SHAP analysis
- MLflow experiment tracking

The current baseline pipeline is infrastructure-complete, though the present sample dataset still shows strong label imbalance, so target design remains an active research task.

### API and dashboard

The project now includes a lightweight inspection layer for demos and validation:

- FastAPI backend for replay summary and feature sampling
- React + Vite dashboard for local visualization
- summary cards for dataset-level metrics
- charts for spread, mid-price, depth, and imbalance
- Docker Compose support for local API/demo startup

---

## Hyperliquid workflow

A small Hyperliquid research pipeline is now part of the project workflow.

Current approach:

1. Extract raw order-status data from downloaded archives.
2. Map raw `statusId` values through lookup tables when available.
3. Parse Hyperliquid-style CSV rows into `ExternalOrderEvent` values.
4. Replay those events through the deterministic replay runner and adapter layer.
5. Export book-derived microstructure features to CSV.
6. Load those features into the Python research layer for model training and evaluation.
7. Inspect replay outputs through the API and dashboard.

Example current CSV shape:

```text
ts,limitPx,sz,isAsk,statusId
2025-12-15 11:39:39.722049503,89691.0,0.01672,True,3
```

Important limitation:

- the current sample is useful for replay experiments, feature generation, dashboard inspection, and ML pipeline validation,
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

## Research workflow

### 1. Export feature CSV from replayable Hyperliquid data

#### Windows PowerShell
```powershell
.\build\Debug\feature_export_main.exe --input data\btc_orders_sample_2025-12-15-12.csv --output output\features.csv --symbol BTCUSDT.P --snapshot-depth 10 --imbalance-depth 10 --ofi-depth 10 --rolling-window 50
```

#### macOS / Linux
```bash
./build/feature_export_main --input data/btc_orders_sample_2025-12-15-12.csv --output output/features.csv --symbol BTCUSDT.P --snapshot-depth 10 --imbalance-depth 10 --ofi-depth 10 --rolling-window 50
```

### 2. Train a baseline with chronological holdout

#### Windows PowerShell
```powershell
$env:PYTHONPATH = "python"
python python/ml/train.py --features-csv output\features.csv --label-type classification --horizon-events 50 --up-threshold 0.0 --down-threshold 0.0
```

#### macOS / Linux
```bash
PYTHONPATH=python python python/ml/train.py --features-csv output/features.csv --label-type classification --horizon-events 50 --up-threshold 0.0 --down-threshold 0.0
```

### 3. Run walk-forward validation with MLflow and SHAP

#### Windows PowerShell
```powershell
$env:PYTHONPATH = "python"
$env:MLFLOW_TRACKING_URI = "sqlite:///mlruns.db"
python python/ml/train.py --features-csv output\features.csv --label-type classification --horizon-events 50 --up-threshold 0.0 --down-threshold 0.0 --validation walk_forward --wf-initial-train-size 50000 --wf-test-size 10000 --wf-step-size 10000 --wf-max-folds 5 --enable-mlflow --mlflow-experiment bookforge --enable-shap --shap-sample-size 2000
```

#### macOS / Linux
```bash
PYTHONPATH=python MLFLOW_TRACKING_URI=sqlite:///mlruns.db python python/ml/train.py --features-csv output/features.csv --label-type classification --horizon-events 50 --up-threshold 0.0 --down-threshold 0.0 --validation walk_forward --wf-initial-train-size 50000 --wf-test-size 10000 --wf-step-size 10000 --wf-max-folds 5 --enable-mlflow --mlflow-experiment bookforge --enable-shap --shap-sample-size 2000
```

### 4. Launch the MLflow UI

```bash
mlflow server --port 8080 --backend-store-uri sqlite:///mlruns.db
```

Then open `http://localhost:8080`.

---

## Local demo

### 1. Start the API

#### Windows PowerShell
```powershell
$env:PYTHONPATH = "python"
uvicorn python.api.main:app --reload --port 8010
```

#### macOS / Linux
```bash
PYTHONPATH=python uvicorn python.api.main:app --reload --port 8010
```

### 2. Start the dashboard

```bash
cd dashboard
npm install
npm run dev
```

Then open the local Vite URL, usually `http://localhost:5173`.

### 3. Optional Docker Compose flow

```bash
docker compose up --build
```

A common Docker pattern is to keep separate images for backend and frontend and connect them with `docker-compose`, which matches the current project layout using a dedicated API Dockerfile and a composed local demo setup.[web:2709]

---

## Replay development

The current replay pipeline is centered on deterministic external event playback.

Main components:

- `ExternalOrderEvent.hpp` — external replay event model
- `HyperliquidCsvReader.*` — parser for Hyperliquid CSV samples
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
8. Run reproducible training/evaluation commands when ML-related
9. Validate the API/dashboard if the change affects exported outputs
10. Commit small, focused changes

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
- [x] Sample CSV generation
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
- [x] OFI
- [x] Depth imbalance
- [x] Spread and mid-price tracking
- [x] Feature export pipeline
- [x] pybind11 bridge

### Phase 5 — Research layer
- [ ] Kyle’s Lambda
- [x] Label generation
- [x] XGBoost training
- [x] Walk-forward evaluation
- [x] SHAP analysis
- [x] ML experiment tracking
- [x] Pytest coverage for Python wrappers

### Phase 6 — Demo layer
- [x] FastAPI backend
- [x] Dashboard
- [ ] Book snapshot inspection endpoint
- [x] End-to-end local demo
- [x] Docker setup

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
- **feature extraction and ML experimentation**
- **walk-forward evaluation and experiment tracking**
- **API-backed inspection and visualization**
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
- The current baseline ML pipeline is operational, but label design and class balance still need improvement for more meaningful predictive evaluation.

---

## Author

Bookforge is developed and maintained by:

- **Dong Quan Tran (Johnny)**
- Role: Owner / Collaborator
- Email: dxt9721@mavs.uta.edu / dongquan.tran.johnny@gmail.com
- GitHub: dong-quan-tran
