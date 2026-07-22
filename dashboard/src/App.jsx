import { useEffect, useMemo, useState } from "react";
import {
  Area,
  AreaChart,
  CartesianGrid,
  Legend,
  Line,
  LineChart,
  ResponsiveContainer,
  Tooltip,
  XAxis,
  YAxis,
} from "recharts";

const API_BASE = import.meta.env.VITE_API_BASE ?? "http://localhost:8010";
const DEFAULT_FEATURES_CSV = "output/features.csv";

function SummaryCard({ label, value }) {
  return (
    <div className="card">
      <div className="card-label">{label}</div>
      <div className="card-value">{value ?? "—"}</div>
    </div>
  );
}

function ChartCard({ title, children }) {
  return (
    <section className="panel">
      <h2>{title}</h2>
      <div className="chart-wrap">{children}</div>
    </section>
  );
}

function fmt(value, digits = 6) {
  return Number.isFinite(value) ? value.toFixed(digits) : "—";
}

export default function App() {
  const [summary, setSummary] = useState(null);
  const [sample, setSample] = useState(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      try {
        setLoading(true);
        setError("");

        const summaryUrl =
          `${API_BASE}/api/replay/summary?features_csv=${encodeURIComponent(DEFAULT_FEATURES_CSV)}`;
        const sampleUrl =
          `${API_BASE}/api/features/sample?features_csv=${encodeURIComponent(DEFAULT_FEATURES_CSV)}&limit=500`;

        const summaryResp = await fetch(summaryUrl);
        const sampleResp = await fetch(sampleUrl);

        if (!summaryResp.ok) {
          const text = await summaryResp.text();
          throw new Error(`Summary request failed: ${summaryResp.status} ${text}`);
        }

        if (!sampleResp.ok) {
          const text = await sampleResp.text();
          throw new Error(`Sample request failed: ${sampleResp.status} ${text}`);
        }

        const summaryJson = await summaryResp.json();
        const sampleJson = await sampleResp.json();

        setSummary(summaryJson);
        setSample(sampleJson);
      } catch (err) {
        console.error(err);
        setError(err.message || "Failed to load dashboard data");
      } finally {
        setLoading(false);
      }
    }

    load();
  }, []);

  const chartData = useMemo(() => {
    if (!sample || !Array.isArray(sample.rows)) return [];

    return sample.rows
      .map((row) => {
        const v = row?.values ?? {};

        const midPrice = Number(v.mid_price);
        const spread = Number(v.spread);
        const l1BidQty = Number(v.l1_bid_qty);
        const l1AskQty = Number(v.l1_ask_qty);
        const lNBidQty = Number(v.lN_bid_qty_sum);
        const lNAskQty = Number(v.lN_ask_qty_sum);
        const imbalance = Number(v.l1_depth_imbalance);
        const ofiL1 = Number(v.ofi_l1);
        const ofiLN = Number(v.ofi_lN);

        return {
          idx: v.replay_event_index ?? row.row_index ?? 0,
          mid_price: Number.isFinite(midPrice) ? midPrice : null,
          spread: Number.isFinite(spread) ? spread : null,
          bid_depth: Number.isFinite(l1BidQty) ? l1BidQty : null,
          ask_depth: Number.isFinite(l1AskQty) ? l1AskQty : null,
          total_bid_depth: Number.isFinite(lNBidQty) ? lNBidQty : null,
          total_ask_depth: Number.isFinite(lNAskQty) ? lNAskQty : null,
          depth_imbalance: Number.isFinite(imbalance) ? imbalance : null,
          ofi_l1: Number.isFinite(ofiL1) ? ofiL1 : null,
          ofi_lN: Number.isFinite(ofiLN) ? ofiLN : null,
        };
      })
      .filter(
        (row) =>
          row.mid_price !== null ||
          row.spread !== null ||
          row.bid_depth !== null ||
          row.ask_depth !== null ||
          row.depth_imbalance !== null ||
          row.ofi_l1 !== null
      );
  }, [sample]);

  if (loading) {
    return (
      <main className="app">
        <div className="status">Loading dashboard…</div>
      </main>
    );
  }

  if (error) {
    return (
      <main className="app">
        <div className="status error">{error}</div>
      </main>
    );
  }

  return (
    <main className="app">
      <header className="hero">
        <div>
          <h1>Bookforge Dashboard</h1>
          <p>Replay summary and feature inspection for exported market microstructure data.</p>
        </div>
      </header>

      <section className="summary-grid">
        <SummaryCard label="Symbol" value={summary?.symbol} />
        <SummaryCard label="Rows" value={summary?.row_count} />
        <SummaryCard label="Columns" value={summary?.column_count} />
        <SummaryCard label="Avg spread" value={fmt(summary?.avg_spread)} />
        <SummaryCard label="Avg mid-price" value={fmt(summary?.avg_mid_price)} />
        <SummaryCard label="Avg imbalance" value={fmt(summary?.avg_depth_imbalance)} />
      </section>

      {chartData.length === 0 ? (
        <section className="panel">
          <h2>No plottable rows yet</h2>
          <p>
            The sampled feature rows are mostly null in the charted columns. Try fetching a later
            slice from the API or adjusting the backend sample strategy.
          </p>
        </section>
      ) : (
        <div className="panel-grid">
          <ChartCard title="Mid-price">
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="mid_price" stroke="#0f766e" dot={false} connectNulls />
              </LineChart>
            </ResponsiveContainer>
          </ChartCard>

          <ChartCard title="Spread">
            <ResponsiveContainer width="100%" height={280}>
              <AreaChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Area type="monotone" dataKey="spread" stroke="#1d4ed8" fill="#93c5fd" connectNulls />
              </AreaChart>
            </ResponsiveContainer>
          </ChartCard>

          <ChartCard title="L1 bid vs ask depth">
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="bid_depth" stroke="#15803d" dot={false} connectNulls />
                <Line type="monotone" dataKey="ask_depth" stroke="#b91c1c" dot={false} connectNulls />
              </LineChart>
            </ResponsiveContainer>
          </ChartCard>

          <ChartCard title="L1 depth imbalance">
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="depth_imbalance" stroke="#7c3aed" dot={false} connectNulls />
              </LineChart>
            </ResponsiveContainer>
          </ChartCard>

          <ChartCard title="OFI L1">
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="ofi_l1" stroke="#c2410c" dot={false} connectNulls />
              </LineChart>
            </ResponsiveContainer>
          </ChartCard>

          <ChartCard title="LN total depth">
            <ResponsiveContainer width="100%" height={280}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="idx" />
                <YAxis domain={["auto", "auto"]} />
                <Tooltip />
                <Legend />
                <Line type="monotone" dataKey="total_bid_depth" stroke="#0369a1" dot={false} connectNulls />
                <Line type="monotone" dataKey="total_ask_depth" stroke="#be123c" dot={false} connectNulls />
              </LineChart>
            </ResponsiveContainer>
          </ChartCard>
        </div>
      )}

      <section className="panel">
        <h2>Recent rows</h2>
        <div className="table-wrap">
          <table>
            <thead>
              <tr>
                <th>Event</th>
                <th>Mid-price</th>
                <th>Spread</th>
                <th>L1 bid qty</th>
                <th>L1 ask qty</th>
                <th>L1 imbalance</th>
                <th>OFI L1</th>
              </tr>
            </thead>
            <tbody>
              {chartData.slice(0, 12).map((row) => (
                <tr key={row.idx}>
                  <td>{row.idx}</td>
                  <td>{row.mid_price ?? "—"}</td>
                  <td>{row.spread ?? "—"}</td>
                  <td>{row.bid_depth ?? "—"}</td>
                  <td>{row.ask_depth ?? "—"}</td>
                  <td>{row.depth_imbalance ?? "—"}</td>
                  <td>{row.ofi_l1 ?? "—"}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </section>
    </main>
  );
}