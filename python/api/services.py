from __future__ import annotations

from pathlib import Path
from typing import Any

import pandas as pd
from fastapi import HTTPException

DEFAULT_FEATURES_CSV = Path("output/features.csv")
DEFAULT_SNAPSHOTS_CSV = Path("output/snapshots.csv")


def _resolve_existing_file(path_str: str | None, default_path: Path) -> Path:
    path = Path(path_str) if path_str else default_path
    if not path.exists():
        raise HTTPException(status_code=404, detail=f"File not found: {path}")
    if not path.is_file():
        raise HTTPException(status_code=400, detail=f"Not a file: {path}")
    return path


def read_csv(path_str: str | None, default_path: Path) -> tuple[Path, pd.DataFrame]:
    path = _resolve_existing_file(path_str, default_path)
    try:
        df = pd.read_csv(path)
    except Exception as exc:
        raise HTTPException(status_code=400, detail=f"Failed to read CSV {path}: {exc}") from exc
    if df.empty:
        raise HTTPException(status_code=400, detail=f"CSV is empty: {path}")
    return path, df


def _first_present(df: pd.DataFrame, candidates: list[str]) -> str | None:
    for col in candidates:
        if col in df.columns:
            return col
    return None


def _safe_mean(df: pd.DataFrame, column: str) -> float | None:
    if column not in df.columns:
        return None
    series = pd.to_numeric(df[column], errors="coerce").dropna()
    if series.empty:
        return None
    return float(series.mean())


def build_replay_summary(features_csv: str | None) -> dict[str, Any]:
    path, df = read_csv(features_csv, DEFAULT_FEATURES_CSV)

    event_col = _first_present(df, ["replay_event_index", "event_index"])
    ts_col = _first_present(df, ["replay_timestamp_ns", "timestamp_ns", "ts_ns"])
    symbol_col = _first_present(df, ["symbol"])
    imbalance_col = _first_present(
        df,
        ["depth_imbalance", "imbalance", "l1_depth_imbalance"],
    )

    symbol = None
    if symbol_col is not None and not df[symbol_col].dropna().empty:
        symbol = str(df[symbol_col].dropna().iloc[0])

    return {
        "features_csv": str(path),
        "row_count": int(len(df)),
        "column_count": int(len(df.columns)),
        "columns": [str(c) for c in df.columns.tolist()],
        "symbol": symbol,
        "start_event_index": int(df[event_col].iloc[0]) if event_col else None,
        "end_event_index": int(df[event_col].iloc[-1]) if event_col else None,
        "start_timestamp_ns": int(df[ts_col].iloc[0]) if ts_col else None,
        "end_timestamp_ns": int(df[ts_col].iloc[-1]) if ts_col else None,
        "avg_spread": _safe_mean(df, "spread"),
        "avg_mid_price": _safe_mean(df, "mid_price"),
        "avg_depth_imbalance": _safe_mean(df, imbalance_col) if imbalance_col else None,
    }


def build_feature_sample(
    features_csv: str | None,
    limit: int,
    offset: int = 0,
) -> dict[str, Any]:
    path, df = read_csv(features_csv, DEFAULT_FEATURES_CSV)

    if limit < 1 or limit > 5000:
        raise HTTPException(status_code=400, detail="limit must be between 1 and 5000")
    if offset < 0:
        raise HTTPException(status_code=400, detail="offset must be >= 0")

    sliced = df.iloc[offset : offset + limit].reset_index(drop=True)
    rows = [
        {"row_index": int(offset + i), "values": row}
        for i, row in enumerate(sliced.to_dict(orient="records"))
    ]

    return {
        "features_csv": str(path),
        "row_count": int(len(df)),
        "returned_count": int(len(rows)),
        "rows": rows,
    }


def build_snapshot_sample(
    snapshot_csv: str | None,
    limit: int,
    offset: int = 0,
) -> dict[str, Any]:
    path, df = read_csv(snapshot_csv, DEFAULT_SNAPSHOTS_CSV)

    if limit < 1 or limit > 5000:
        raise HTTPException(status_code=400, detail="limit must be between 1 and 5000")
    if offset < 0:
        raise HTTPException(status_code=400, detail="offset must be >= 0")

    sliced = df.iloc[offset : offset + limit].reset_index(drop=True)
    rows = [
        {"row_index": int(offset + i), "values": row}
        for i, row in enumerate(sliced.to_dict(orient="records"))
    ]

    return {
        "snapshot_csv": str(path),
        "row_count": int(len(df)),
        "returned_count": int(len(rows)),
        "rows": rows,
    }


def build_columns_response(features_csv: str | None) -> dict[str, Any]:
    path, df = read_csv(features_csv, DEFAULT_FEATURES_CSV)
    return {
        "features_csv": str(path),
        "columns": [str(c) for c in df.columns.tolist()],
    }