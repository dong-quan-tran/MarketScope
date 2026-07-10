# MarketScope

MarketScope is a hybrid **C++ + Python** market microstructure project built to study how a modern limit order book behaves under real order flow.

The core of the project is a low-latency **C++20 limit order book** with support for order insertion, cancellation, execution, and depth queries. On top of that, the project adds a Python research layer for feature extraction, liquidity estimation, machine learning, and a small API/dashboard demo.

The goal is to build a repo that is both:
- technically strong enough for **quant SWE / quant research interviews**
- practical enough to use for **microstructure experiments and short-horizon signal research**

---

## Project goals

- Build a clean and testable **price-time priority order book**
- Replay historical market events from **LOBSTER-style data**
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

### Python layer
- Python 3.11+
- pybind11
- pandas
- numpy
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

Planned next work:
- LOBSTER message replay
- Matching engine behavior
- Feature exporter
- pybind11 bridge
- Python ML pipeline
- API and dashboard

---

## Repository layout

```text
MarketScope/
├── src/
│   ├── core/          # order, price level, order book
│   ├── features/      # feature extraction from book state
│   ├── matching/      # matching engine logic
│   └── replay/        # historical message replay / snapshot serialization
├── bindings/          # pybind11 bindings
├── python/
│   ├── lob_engine/    # Python wrapper around C++ core
│   ├── ml/            # feature engineering, liquidity metrics, training
│   ├── api/           # FastAPI backend
│   └── dashboard/     # React frontend
├── tests/
│   ├── cpp/           # GoogleTest suites
│   └── python/        # Pytest suites
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

## Getting started

### 1. Clone the repo

```bash
git clone <your-repo-url>
cd MarketScope
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

#### Windows (Visual Studio generator)
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
Use both commands during development:
```bash
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
pytest tests/python -q
```

---

## Development workflow

A good working loop for this repo:

1. Implement a small feature in C++
2. Add or update Google Tests
3. Build and run C++ tests
4. Expose the feature to Python when needed
5. Add Python-side tests
6. Commit small, focused changes

---

## Documentation

Project planning and notes live in `docs/`:

- `docs/BLUEPRINT.md` — master implementation plan
- `docs/ARCHITECTURE.md` — design choices and benchmark notes
- `docs/DATA_GUIDE.md` — data format notes
- `docs/INTERVIEW_PREP.md` — talking points and likely interview questions
- `docs/WEEK_BY_WEEK.md` — development progress log

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
- [ ] Historical event replay
- [ ] Matching engine
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

MarketScope is meant to combine both:
- **systems engineering** through a C++ order book core
- **market microstructure intuition**
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
- Historical data formats and replay logic may evolve as the project grows.

---

## License

Add a license before publishing publicly. MIT is a common default for portfolio projects.