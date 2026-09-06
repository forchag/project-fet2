# Removed: confirmed data-fabrication scripts

Removed at the user's explicit request during the V10 data-integrity
investigation (paper/v10-real-data-update branch), to eliminate any risk
of these being run again and mistaken for real measurement.

| Removed file | What it did | Evidence |
|---|---|---|
| `generate_peer_scaling_campaign.py` | Generated peer-scaling latency and `rbac_overhead_ms` via `numpy.random.default_rng(...).normal(...)`, seeded with the manuscript's own target means (`TOTAL_MEAN_MS`, `SPAN_MEAN_MS=319.6`) and `SAMPLES_PER_RUN=130`. See `paper_iot/V10/analysis/PEER_SCALING_FABRICATION_FINDING.md`. |
| `generate_interleaved_throughput_campaign.py` | Overwrote `data/raw/raw_throughput_samples.csv` entirely via the same RNG pattern; its own docstring said it "draws every run as a fresh, independent value at generation time." |
| `crt_load_test.py` | Self-titled "CRT Synthetic Load Injection Test"; its `extract_sandstorm_day47()` / `extract_peak_slot_util()` functions claimed to be "extracted from Prometheus deployment logs" / "extracted from raw_sensor_transactions.csv" but returned hardcoded constants. |
| `interleave_throughput_runs.py` | Already a dead stub before removal; its own docstring documented an earlier round of the same pattern (relabeling old timestamps to look interleaved without new measurements). |

All four are recoverable from git history (`git log --diff-filter=D -- scripts/<name>`) if anyone needs to see exactly what they did; they should not be restored to a runnable state.

## What real instrumentation replaces them

See `chaincode/hrbac/timing.go` and `gateway/gateway.py` for the actual
timing instrumentation added to measure `rbac_overhead_ms` for real, and
`analysis/run_campaign.py` for the live-network peer-scaling/throughput
driver that records genuine measurements instead.
