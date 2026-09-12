# Rule 8 stop report — peer-scaling latency campaign

CLAUDE.md rule 8: "If you find evidence that any trace was generated, simulated,
synthesised, reconstructed, interpolated or transformed rather than captured in the
field, stop all manuscript editing and report to the author... Do not soften the
finding, do not keep editing prose, do not propose wording that hides it."

That evidence exists, in the repository's own history, for the peer-scaling latency
campaign that underlies Section 6.1.1 (the "latency falls as peers scale from 4 to 32"
result) and the `rbac_overhead_ms` / "recorded authorization-associated span" figure
used throughout the manuscript, including in the abstract-level claims. No manuscript
text has been edited. This report is the complete evidence chain; the LaTeX is
untouched.

## 1. A prior investigation already found this dataset fabricated, in writing, in this repo

Commit `a16e25b3` ("Clarify real-data provenance and merged-folder origin", authored
2026-09-06 20:24:25 +0100 by the paper's own author) deleted 68 of 77 lines from
`paper_iot/V10/analysis/PEER_SCALING_FABRICATION_FINDING.md`. The deleted text — recovered
verbatim with `git show a16e25b -- '*/PEER_SCALING_FABRICATION_FINDING.md'` — read:

> **Status: confirmed, not circumstantial.**
>
> `scripts/generate_peer_scaling_campaign.py` ... contains:
> ```python
> SAMPLES_PER_RUN = 130
> TOTAL_MEAN_MS = {4: 1219.8, 8: 1147.1, 16: 1050.5, 32: 820.5}
> SPAN_MEAN_MS = 319.6
> TOTAL_BETWEEN_RUN_SD = 22.0
> TOTAL_WITHIN_RUN_SD = 55.0
> SPAN_BETWEEN_RUN_SD = 4.5
> SPAN_WITHIN_RUN_SD = 27.0
> ...
> run_total_mean = rng.normal(TOTAL_MEAN_MS[peers], TOTAL_BETWEEN_RUN_SD)
> run_span_mean = rng.normal(SPAN_MEAN_MS, SPAN_BETWEEN_RUN_SD)
> ...
> span = max(1.0, rng.normal(run_span_mean, SPAN_WITHIN_RUN_SD))
> total = max(span + 1.0, rng.normal(run_total_mean, TOTAL_WITHIN_RUN_SD))
> ```
> `TOTAL_MEAN_MS` and `SPAN_MEAN_MS` are the manuscript's own target numbers, hardcoded
> as the generator's Gaussian means... [and] `chaincode/hrbac/*.go` (`CheckAccess`,
> `WriteSensorData`, every function) contains no `time.Since()`, no timer, and no code
> path that writes an authorization-timing span anywhere. There is nothing in the actual
> deployed smart contract that could have produced `rbac_overhead_ms` as a real
> measurement.

The same commit removed the file's "Consequence" section, which had said: "Do not use
`peer_scaling_transactions.csv`, `peer_scaling_runs.csv`, or any `rbac_overhead_ms`
value from any file in this package as measured evidence."

The companion file `scripts/REMOVED_FABRICATION_SCRIPTS.md` — before commit `7e906da5`
deleted its table — additionally documented three more confirmed-fabrication scripts:
`generate_interleaved_throughput_campaign.py` (overwrote `raw_throughput_samples.csv`
via the same RNG pattern; its own docstring said it "draws every run as a fresh,
independent value at generation time"), `crt_load_test.py` (functions named
`extract_sandstorm_day47()` / `extract_peak_slot_util()` that claimed to be "extracted
from Prometheus deployment logs" / "extracted from raw_sensor_transactions.csv" but
returned hardcoded constants), and `interleave_throughput_runs.py` (a stub whose
docstring documented "relabeling old timestamps to look interleaved without new
measurements").

`paper_iot/V10/analysis/DATA_PROVENANCE_REPORT.md`, before commit `5fff0850` cut it from
95 lines to 27, classified **every single file in the V10 package** as
`provenance_uncertain` — none as `verified_real_field` or `verified_real_controlled` —
and separately flagged that three successive "raw data" deliveries during that
investigation "each fixed exactly the specific gap flagged in the previous one... and
each landed within noise of the manuscript's own Table 3 numbers," that
`sensor_transactions.csv` showed zero jitter through the outage windows with no
buffering signature (contradicting the buffered-replay explanation now in
`results/outage_data_integrity_report.md`, which is built from a *different*, later
V10 "full" package with a different ID scheme), that `authorization_decisions.csv` had
zero `tx_id` overlap with `sensor_transactions.csv`, that the peer-scaling runs were
strictly sequential on a single host rather than the counterbalanced four-host design
the manuscript described, that the throughput runs were blocked (all baseline before
all HRBAC) rather than interleaved, that energy measurements ran in the *opposite*
direction from what the manuscript claimed, that the 50 device logs were byte-identical
to the CSV rows they were supposed to corroborate and covered only day 1, and that the
gateway logs covered only one sensor.

## 2. That documentation was deleted and replaced with unsupported assertions, not refuted

Eleven commits, all on 2026-09-06 between 20:22:51 and 20:29:16 (same session), titled
"Clarify real-data provenance and merged-folder origin" or "Remove remaining
artificial-data wording," rewrote every file above (and `README.md`,
`analysis/README.md`, `data/raw/README.md`, `paper_iot/dist/README.md`, two
`RESPONSE-TO-PREMORTEM*.md` files) to assert "these are real measured data" and
attribute every discrepancy to "merging two source folders." None of the eleven commits
introduces new evidence — no re-run, no independent log, no counter-analysis. They
replace the quoted generator code and the file-by-file findings with prose that never
engages the specific evidence it deletes (the `rng.normal()` call keyed to
`TOTAL_MEAN_MS`, the absent chaincode instrumentation, the zero `tx_id` overlap, the
byte-identical single-day logs). This is exactly rule 8's "do not soften the finding, do
not propose wording that hides it," already executed once against this dataset by a
prior session under the same instructions.

## 3. The current, shipped `data/raw/raw_latency_samples.csv` still carries this signature

`data/benchmarks/README.md` (current, unmodified by this session) states in its own
"Data-validity notes (V08)" table: *"`raw_latency_samples.csv` peer-scaling rows
(`concurrent_clients == 50`) | Reprocessed for V08 by
`scripts/generate_peer_scaling_campaign.py`"* — naming the exact script quoted above as
the source of the file the manuscript now cites as field-measured. The script itself is
gone from the working tree (removed per `scripts/REMOVED_FABRICATION_SCRIPTS.md`) and
does not appear as a blob anywhere in `git log --all --diff-filter=A` for that path, so
its exact current parameters cannot be re-inspected — but the current data match its
quoted 2026-vintage constants closely enough to be its output or a close descendant:

| peer_count | n | mean `latency_ms` | sd | mean `rbac_overhead_ms` | sd |
|---|---|---|---|---|---|
| 4  | 1540 | 1219.9 | 59.1 | 321.2 | 26.8 |
| 8  | 1040 | 1149.5 | 58.3 | 320.4 | 27.4 |
| 16 | 1040 | 1056.9 | 59.1 | 322.9 | 27.7 |
| 32 | 1040 |  811.8 | 58.2 | 316.8 | 27.1 |

against the deleted finding's quoted generator constants `TOTAL_MEAN_MS = {4: 1219.8,
8: 1147.1, 16: 1050.5, 32: 820.5}`, `SPAN_MEAN_MS = 319.6`. The 4-peer mean matches to
0.1 ms. The `rbac_overhead_ms` field being present at all is itself notable: the V10
provenance report (before it was softened) listed `rbac_overhead_ms` as `missing`
"entirely" from both raw files at that stage, and the un-softened chaincode-scan finding
says no timer in `chaincode/hrbac/*.go` ever wrote such a field. (Computed with
`python3` over `data/raw/raw_latency_samples.csv`, this session, reproducible.)

## What this means for the manuscript

- Section 6.1.1's peer-scaling latency result (the "495 ms + 388 ms at 4 peers falling
  to 495 ms at 32 peers" / current macro-derived 1220→812 ms narrative) and every use of
  `rbac_overhead_ms` (the "~320 ms recorded authorization-associated span," including
  the 280 ms-vs-320 ms inconsistency the desk reviewer flagged) rest on a field the
  paper's own repository history says was synthesized from Gaussian draws centered on
  the manuscript's target numbers, with no chaincode timer that could have produced it.
- Per rule 8 this is not something I can write an explanation for, qualify in a footnote,
  or launder through a "processing" narrative — that is the exact move the deleted
  commits already made, and it is what CLAUDE.md instructs me not to repeat.
- I have not touched `chaincode/hrbac/*.go`, `paper_iot/V14/hrbac_iot_cas_dc.tex`, or any
  other manuscript file. `AUDIT/raw_hashes_before.txt` is written; the raw CSVs
  themselves are untouched.

## Open questions for the author

1. Is `paper_iot/V10/analysis/generate_peer_scaling_campaign.py` (or its output)
   genuinely how `data/raw/raw_latency_samples.csv`'s peer-scaling rows and
   `rbac_overhead_ms` column were produced? If there is a real instrumentation source
   instead (e.g. `chaincode/hrbac/timing.go`, mentioned in the current
   `scripts/REMOVED_FABRICATION_SCRIPTS.md` as the real replacement), where is the
   raw output of *that* path, separate from this file?
2. Who ran the eleven "Clarify real-data provenance" commits, and on what basis — was
   new evidence found that isn't in the repository, or was this a decision to change how
   the data is described without new evidence?
3. Does the same question apply to `raw_throughput_samples.csv` (named in the same
   removed-scripts table, via `generate_interleaved_throughput_campaign.py`) and to
   `raw_sensor_transactions.csv` / the outage-buffering story (the V10 provenance report
   flagged zero jitter through outages before a later "full package" replaced it with a
   buffered-replay explanation using different sensor and gateway IDs)? I have not yet
   re-run those specific checks against current `data/raw/` under rule 8 conditions —
   I stopped as soon as the peer-scaling evidence was clear, per the rule.
4. Given this, which of the paper types in Phase 1.11 (A/B/C) do you want me to work
   toward? A field-audit reframing (Type A) is very unlikely to survive contact with
   this evidence for the peer-scaling result specifically; Type C ("core traces not
   field-captured... list what would be needed instead") looks like the honest starting
   point unless you can point me at the real instrumentation output.

I'm stopping here, as instructed, rather than continuing to Phase 0.3 onward or drafting
any manuscript language.
