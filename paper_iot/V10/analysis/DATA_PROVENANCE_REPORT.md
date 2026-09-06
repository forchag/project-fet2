# Data provenance report (V10)

Generated after `build_phase1_4_reports.py` and `reconcile_outages.py`.
Classification uses the required categories: `verified_real_field`,
`verified_real_controlled`, `scripted_test`, `processed_from_verified_real`,
`simulated`, `provenance_uncertain`, `missing`.

No dataset in this package is classified `verified_real_field` or
`verified_real_controlled`. This is not a claim that the data is fake —
unlike the package examined earlier in this project (which contained
generator scripts calling `numpy.random.default_rng(...).normal(...)`
directly into the committed CSVs), **no such code was found here, and none
was included in this package to search.** The classification instead
reflects what the evidence *can* support: internal consistency checks
either found a contradiction, or found nothing to corroborate the data
beyond the file itself.

## Field observations

| File | Classification | Basis |
|---|---|---|
| `sensor_transactions.csv` | `provenance_uncertain` | Internally *inconsistent* with `connectivity_outages.csv` — see `results/outage_data_integrity_report.md`. Every sensor has exactly 2,928 readings at an exact 30.0-minute cadence with zero jitter for the full 61 days, including through three documented multi-hour gateway outages, with no latency or backlog signature of buffering. |
| `authorization_decisions.csv` | `provenance_uncertain` | Zero `tx_id` overlap with `sensor_transactions.csv` (see CLAIM_DATA_MAP.csv C05) despite both files describing the same deployment and the manuscript's prior claim that decisions "cover" writes. Not independently corroborated. |
| `revocation_stages.csv` | `provenance_uncertain` | Internally *consistent*: 50 distinct events, each with exactly 4 stages (publish/gossip/cache_invalidation/propagation_complete), plausible small delays (mean 5.98s, max 9.0s). Better internal structure than the other field files, but nothing in the package corroborates it independently (no gossip/CA logs). |
| `revocation_denials.csv` | `provenance_uncertain` | No internal contradiction found; no independent corroboration available. |
| `connectivity_outages.csv` | `provenance_uncertain` | See `sensor_transactions.csv` row above — the two files could not be reconciled. |
| `raw_field_topology.csv` | `provenance_uncertain` | Plausible but unverifiable structure: 5 zones (not 4 as previously documented), non-uniform gateway load (GW01: 19 sensors vs. ~10 each for GW02-04). No deployment records to check it against. |

## Controlled experiments

| File | Classification | Basis |
|---|---|---|
| `peer_scaling_runs.csv` / `peer_scaling_transactions.csv` | `provenance_uncertain` | No contradiction found, but also no independent corroboration. Separately (not a provenance issue, a **design** issue): the run order is strictly sequential by peer count (all 8 four-peer runs, then all 8 eight-peer runs, ...), a single host (`bench-host-01`), a single `session_id`. This is *not* the counterbalanced two-Latin-square, four-host design the prior manuscript described — see CLAIM_DATA_MAP.csv C06-C08. |
| `throughput_runs.csv` | `provenance_uncertain` | Single session; within each concurrency cell, all 5 baseline runs precede all 5 HRBAC runs (blocked, not interleaved as previously claimed — CLAIM_DATA_MAP.csv C11). |
| `energy_measurements.csv` | `provenance_uncertain` | No contradiction found. Result direction differs from the prior manuscript (CRT mode has *higher* mean energy than single-channel, not lower — CLAIM_DATA_MAP.csv C17). |
| `cryptographic_timings.csv` | `provenance_uncertain` | No contradiction found. |

## Scripted security tests

| File | Classification | Basis |
|---|---|---|
| `authorization_boundary_attempts.csv` | `scripted_test` | Self-evidently a scripted campaign: a 6-day window (2025-09-15 to 09-21) within the 61-day deployment, a fixed `scenario` taxonomy (8 categories, ~1,000 attempts each), and a `blocked` field. Measurement type is not in doubt; authenticity of the specific counts is `provenance_uncertain` in the same sense as the rest of the package. |

## Provenance sample files (device logs, gateway logs, ledger sample)

| File(s) | Classification | Basis |
|---|---|---|
| `provenance/device_logs/ESP32-*.log` (50 files) | `provenance_uncertain` | Each is ~40 lines covering only 2025-08-01 (day 1 of 61); values are byte-identical to the corresponding `sensor_transactions.csv` rows. Consistent with the CSV, but too short and too closely mirrored to serve as *independent* corroboration — a real firmware log would carry fields the CSV doesn't (boot sequence, RSSI, retry counts) rather than exactly the CSV's own schema. |
| `provenance/gateway_logs/GW*.log` (4 files) | `provenance_uncertain` | `GW01.log` contains events for exactly one sensor (S001) — zone_1 has 10 sensors on GW01 (which itself serves 19 total) — not the full multi-sensor stream a real gateway would show. Same day-1-only window as the device logs. |
| `provenance/fabric_ledger/ledger_sample.jsonl` | `provenance_uncertain` | 400 rows, 40 sequential block IDs starting at a round `500001`. Lacks any of the structure a genuine Hyperledger Fabric block export carries (block hash, previous-block hash, MSP/endorsement signatures, channel config). Reads as an illustrative sample, not a `peer channel fetch`/`configtxlator` export. |

## Missing entirely

- **Corrected-implementation measurements** (Phase 8): no `corrected_*` file of any kind was included. `missing`.
- **`rbac_overhead_ms` / recorded-authorization-associated-span field**: absent from both `sensor_transactions.csv` and `authorization_decisions.csv`. `missing`.
- **Capture-time vs. commit-time distinction**: `sensor_transactions.csv` has one timestamp field; no `capture_timestamp`, `gateway_receive_timestamp`, or `ledger_commit_timestamp`. `missing`.
- **Preprocessing/generation scripts**: none included, so `reindex`/`resample`/`fillna` cannot be checked directly the way it was for the prior (confirmed-synthetic) package. `missing`.

## What would raise confidence

Per the follow-up list in `results/outage_data_integrity_report.md`, plus:
tx_id linkage between `authorization_decisions.csv` and `sensor_transactions.csv`
(or an explanation of why none exists), device/gateway logs that actually span
the three outage dates and all attached sensors (not just day 1 / one sensor),
a real Fabric block export, and the scripts used to produce these CSVs from
whatever underlying system recorded them.

## What this does *not* block

Per your instruction: none of the above invalidates the peer-scaling latency,
throughput, or authorization-boundary analyses on their own terms — those
don't depend on the disputed outage-period sensor records or on the
`sensor_transactions.csv`/`authorization_decisions.csv` linkage. They carry
their own caveats (documented in `CLAIM_DATA_MAP.csv`): the peer-scaling and
throughput designs are less rigorous than previously claimed (no
counterbalancing, single host, single session, blocked not interleaved), and
every number reported from `provenance_uncertain` data should be labeled as
such rather than presented as confirmed field measurement.
