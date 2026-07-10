# Bookforge � Project Blueprint & TODO

## Project Summary
Low-latency C++20 limit order book with pybind11 Python bridge, multi-level
Order Flow Imbalance (OFI) feature extraction, Kyle's Lambda liquidity
estimation, and an XGBoost ML layer to predict short-term mid-price direction.

## WEEK 1 � C++ Order Book Core
- [ ] Define Order struct in src/core/order.hpp
- [ ] Implement PriceLevel in src/core/price_level.hpp/.cpp
- [ ] Implement OrderBook in src/core/order_book.hpp/.cpp
- [ ] Add AddOrder(), CancelOrder(), ExecuteOrder()
- [ ] Add GetBestBid(), GetBestAsk(), GetMidPrice(), GetSpread()
- [ ] Write 20+ Google Test cases
- [ ] Add latency benchmark
- [ ] Record results in docs/ARCHITECTURE.md

## WEEK 2 � LOBSTER Replayer + Matching Engine
- [ ] Download LOBSTER sample data
- [ ] Implement MessageReplayer
- [ ] Implement MatchingEngine
- [ ] Implement SnapshotSerializer
- [ ] Replay real data end-to-end
- [ ] Verify mid-price against LOBSTER snapshots

## WEEK 3 � Feature Exporter + pybind11
- [ ] Implement OFI levels 1�5
- [ ] Add weighted OFI, spread, imbalance
- [ ] Expose C++ book + features with pybind11
- [ ] Build Python wrapper
- [ ] Implement Kyle's Lambda
- [ ] Write Pytest cases

## WEEK 4 � ML Signal Layer
- [ ] Build label generator
- [ ] Assemble feature set
- [ ] Train XGBoost classifier
- [ ] Add walk-forward validation
- [ ] Evaluate accuracy, precision/recall, Sharpe
- [ ] Add SHAP feature importance
- [ ] Log runs with MLflow

## WEEK 5 � FastAPI + Dashboard
- [ ] Build FastAPI endpoints
- [ ] Add API tests
- [ ] Build React dashboard
- [ ] Add latency benchmark script
- [ ] Add Docker setup

## WEEK 6 � Polish
- [ ] Reach 50+ Google Test cases
- [ ] Reach 50+ Pytest cases
- [ ] Add GitHub Actions CI
- [ ] Finish README
- [ ] Finish docs/INTERVIEW_PREP.md
- [ ] Tag v1.0.0
