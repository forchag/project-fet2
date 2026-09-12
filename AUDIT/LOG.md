# AUDIT/LOG.md

## 2026-09-12 — Phase 0 start, rule 8 triggered before Phase 0 completed

### Setup
- Confirmed working branch: `claude/paper-v16-revision-pf4am6` (harness-assigned; CLAUDE.md
  rule 10 names `audit-revision` — flagged as a conflict, harness branch used since that is
  how this session's work is tracked/pushed; author should say if a rename is wanted).
- `sha256sum data/raw/* > AUDIT/raw_hashes_before.txt` — done, 10 raw files + README.md +
  VALIDATION.md hashed, row counts recorded in the same file.
- Located the Elsevier CAS double-column manuscript source: `paper_iot/V14/hrbac_iot_cas_dc.tex`
  (1,910 lines). **No V15 or V16 `.tex` exists anywhere in the repository** — searched
  `grep -rl "V16\|V15" --include=*.tex --include=*.md .` outside `lit-rev/` and the V10-V14
  version directories: no hits. The V16 PDF the author supplied has no corresponding tracked
  source; V14 is the latest tex. `project fin update with reviewers/main_article.tex` is a
  different manuscript entirely (Springer Nature `sn-jnl` class, not Elsevier CAS) — a sibling
  submission on the same deployment, not this paper's V15/V16.

### Data-history check (CLAUDE.md 0.5) — found rule 8 evidence

`git log --format='%h %ad %an %s' --date=iso -- data/raw/` shows all 10 raw CSVs and the two
data-dictionary files arrived in a single `ebdc0f2` "Initial commit" (2026-09-06), with one
follow-up wording edit (`743836e`) to `data/raw/README.md` the same day. By itself this is weak
evidence (committing a field dataset once, after write-up, is normal) — noted but not
actionative alone.

Grep for generator patterns (CLAUDE.md 1.1) surfaced `analysis/derive_results.py`,
`analysis/run_campaign.py`, `gateway/fabric_client.py`, `paper_iot/V10/analysis/reconcile_outages.py`,
`tests/performance/workload_generator.py` — none contain a live RNG call reaching `data/raw/`
in their current form (not yet fully traced line-by-line; superseded by the finding below).

Following up on repeated commit messages "Clarify real-data provenance and merged-folder
origin" / "Remove remaining artificial-data wording" (11 occurrences, all `2026-09-06 20:2x`,
same author, same session) turned up **direct, repo-native, dated evidence that a generated
dataset was documented as fabricated, and that documentation was then deleted and replaced
with unsupported "real data" assertions, without new counter-evidence.** Full account in
`AUDIT/RULE8_STOP_REPORT.md`. Rule 8 applies. Stopping all further Phase 0/1 work and manuscript
editing per CLAUDE.md; reporting to the author now.
