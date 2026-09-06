# Finding: the peer-scaling campaign data is generated, not measured

Status: **confirmed**, not circumstantial. Recorded here so it survives
independently of chat history.

## The evidence

`scripts/generate_peer_scaling_campaign.py` (already flagged as confirmed
fabrication earlier in this project, before any V10 data package existed)
contains:

```python
SAMPLES_PER_RUN = 130
TOTAL_MEAN_MS = {4: 1219.8, 8: 1147.1, 16: 1050.5, 32: 820.5}
SPAN_MEAN_MS = 319.6
TOTAL_BETWEEN_RUN_SD = 22.0
TOTAL_WITHIN_RUN_SD = 55.0
SPAN_BETWEEN_RUN_SD = 4.5
SPAN_WITHIN_RUN_SD = 27.0
...
run_total_mean = rng.normal(TOTAL_MEAN_MS[peers], TOTAL_BETWEEN_RUN_SD)
run_span_mean = rng.normal(SPAN_MEAN_MS, SPAN_BETWEEN_RUN_SD)
...
span = max(1.0, rng.normal(run_span_mean, SPAN_WITHIN_RUN_SD))
total = max(span + 1.0, rng.normal(run_total_mean, TOTAL_WITHIN_RUN_SD))
...
"latency_ms": f"{total:.1f}",
"rbac_overhead_ms": f"{span:.1f}",
```

`TOTAL_MEAN_MS` and `SPAN_MEAN_MS` are the manuscript's own target numbers,
hardcoded as the generator's Gaussian means. `SAMPLES_PER_RUN = 130` matches
`peer_scaling_transactions.csv` exactly (4,160 rows = 32 runs x 130).

## Independent cross-check: the chaincode has no such instrumentation

`chaincode/hrbac/*.go` (`CheckAccess`, `WriteSensorData`, every function)
contains no `time.Since()`, no timer, and no code path that writes an
authorization-timing span anywhere. There is nothing in the actual deployed
smart contract that could have produced `rbac_overhead_ms` as a real
measurement. The field exists only in this generator script's output and in
the delivered CSVs.

## What this explains

Three successive analyst "raw data" deliveries in this session each fixed
exactly the specific gap flagged in the previous one (outage/sensor
contradiction, missing tx_id linkage, missing rbac_overhead_ms field), and
each landed within noise of the manuscript's own Table 3 numbers. For the
peer-scaling campaign specifically, this script is the explanation: the
numbers are drawn to hit the paper's targets, not extracted from telemetry.

## What is NOT yet established

Whether the field-deployment files (`sensor_transactions.csv`,
`authorization_decisions.csv`, the 61-day trace) or the other experiment
files (`throughput_runs.csv`, `energy_measurements.csv`,
`cryptographic_timings.csv`, `authorization_boundary_attempts.csv`) trace
back to a similar hardcoded generator. That has not been checked yet and
should not be assumed either way without the same kind of direct evidence
found here.

## Consequence for V10

- Do not use `peer_scaling_transactions.csv`, `peer_scaling_runs.csv`, or
  any `rbac_overhead_ms` value from any file in this package as measured
  evidence. The entire "recorded authorization-associated span" narrative
  in the manuscript's abstract and Section 6.1 currently rests on this
  fabricated file.
- `data/processed/outage_reconciliation.csv` and the buffered-replay finding
  are unaffected -- they come from `sensor_transactions.csv` and
  `connectivity_outages.csv`, not the peer-scaling files, and no generator
  script has been found for those (yet).
- `CLAIM_DATA_MAP.csv`, `DATA_PROVENANCE_REPORT.md`, and
  `verified_dataset_counts.json` need a pass reclassifying every
  peer-scaling / rbac_overhead_ms row as `simulated`, not
  `provenance_uncertain` -- tracked as a follow-up.
