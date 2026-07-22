from pydantic import BaseModel, Field
from typing import Any


class HealthResponse(BaseModel):
    status: str = "ok"


class ReplaySummaryResponse(BaseModel):
    features_csv: str
    row_count: int
    column_count: int
    columns: list[str]
    symbol: str | None = None
    start_event_index: int | None = None
    end_event_index: int | None = None
    start_timestamp_ns: int | None = None
    end_timestamp_ns: int | None = None
    avg_spread: float | None = None
    avg_mid_price: float | None = None
    avg_depth_imbalance: float | None = None


class FeatureSampleRow(BaseModel):
    row_index: int
    values: dict[str, Any]


class FeatureSampleResponse(BaseModel):
    features_csv: str
    row_count: int
    returned_count: int
    rows: list[FeatureSampleRow]


class SnapshotSampleRow(BaseModel):
    row_index: int
    values: dict[str, Any]


class SnapshotSampleResponse(BaseModel):
    snapshot_csv: str
    row_count: int
    returned_count: int
    rows: list[SnapshotSampleRow]


class ColumnsResponse(BaseModel):
    features_csv: str
    columns: list[str]


class ErrorResponse(BaseModel):
    detail: str = Field(..., description="Human-readable error message")