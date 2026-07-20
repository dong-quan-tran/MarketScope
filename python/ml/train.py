from __future__ import annotations

import argparse
import json
from pathlib import Path

import pandas as pd
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    mean_absolute_error,
    mean_squared_error,
    r2_score,
)
from xgboost import XGBClassifier, XGBRegressor

import bookforge_py as bf


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Train first XGBoost baseline for Bookforge features.")
    parser.add_argument("--features-csv", required=True, help="Path to exported feature CSV")
    parser.add_argument(
        "--label-type",
        choices=["classification", "regression"],
        default="classification",
        help="Type of target to train on",
    )
    parser.add_argument(
        "--horizon-events",
        type=int,
        default=50,
        help="Prediction horizon in events",
    )
    parser.add_argument(
        "--up-threshold",
        type=float,
        default=0.0,
        help="Classification up threshold in log-return space",
    )
    parser.add_argument(
        "--down-threshold",
        type=float,
        default=0.0,
        help="Classification down threshold in log-return space",
    )
    parser.add_argument(
        "--train-fraction",
        type=float,
        default=0.8,
        help="Chronological fraction to use for training",
    )
    parser.add_argument(
        "--output-dir",
        default="artifacts/ml_baseline",
        help="Directory to store model and metrics",
    )
    return parser


def _train_classifier(X_train: pd.DataFrame, y_train: pd.Series) -> XGBClassifier:
    n_classes = int(y_train.nunique())

    if n_classes == 2:
        model = XGBClassifier(
            objective="binary:logistic",
            n_estimators=200,
            max_depth=6,
            learning_rate=0.05,
            subsample=0.8,
            colsample_bytree=0.8,
            reg_lambda=1.0,
            random_state=42,
            tree_method="hist",
            eval_metric="logloss",
        )
    else:
        model = XGBClassifier(
            objective="multi:softprob",
            num_class=n_classes,
            n_estimators=200,
            max_depth=6,
            learning_rate=0.05,
            subsample=0.8,
            colsample_bytree=0.8,
            reg_lambda=1.0,
            random_state=42,
            tree_method="hist",
            eval_metric="mlogloss",
        )

    model.fit(X_train, y_train)
    return model

def _train_regressor(X_train: pd.DataFrame, y_train: pd.Series) -> XGBRegressor:
    model = XGBRegressor(
        objective="reg:squarederror",
        n_estimators=200,
        max_depth=6,
        learning_rate=0.05,
        subsample=0.8,
        colsample_bytree=0.8,
        reg_lambda=1.0,
        random_state=42,
        tree_method="hist",
        eval_metric="rmse",
    )
    model.fit(X_train, y_train)
    return model


def main() -> None:
    parser = _build_parser()
    args = parser.parse_args()

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    dataset = bf.build_training_dataset(
        args.features_csv,
        label_type=args.label_type,
        horizon_events=args.horizon_events,
        up_threshold=args.up_threshold,
        down_threshold=args.down_threshold,
    )

    split = bf.chronological_split(
        dataset,
        train_fraction=args.train_fraction,
    )

    if args.label_type == "classification":
        model = _train_classifier(split.X_train, split.y_train)
        y_pred = model.predict(split.X_test)

        metrics = {
            "task": "classification",
            "n_train": int(len(split.X_train)),
            "n_test": int(len(split.X_test)),
            "accuracy": float(accuracy_score(split.y_test, y_pred)),
            "classification_report": classification_report(
                split.y_test,
                y_pred,
                output_dict=True,
                zero_division=0,
            ),
        }

    else:
        model = _train_regressor(split.X_train, split.y_train)
        y_pred = model.predict(split.X_test)

        metrics = {
            "task": "regression",
            "n_train": int(len(split.X_train)),
            "n_test": int(len(split.X_test)),
            "mae": float(mean_absolute_error(split.y_test, y_pred)),
            "rmse": float(mean_squared_error(split.y_test, y_pred) ** 0.5),
            "r2": float(r2_score(split.y_test, y_pred)),
        }

    model_path = output_dir / "xgboost_baseline.json"
    metrics_path = output_dir / "metrics.json"
    feature_columns_path = output_dir / "feature_columns.json"

    model.save_model(model_path.as_posix())

    with metrics_path.open("w", encoding="utf-8") as f:
        json.dump(metrics, f, indent=2)

    with feature_columns_path.open("w", encoding="utf-8") as f:
        json.dump(list(dataset.X.columns), f, indent=2)

    print(json.dumps(metrics, indent=2))


if __name__ == "__main__":
    main()