# Manuscript submission package

This directory contains the revised journal manuscript, supplementary
material, reproducibility sources and the files needed to prepare a Zenodo
research artifact.

## Build

```sh
bash paper_iot/V14/build_v14.sh
```

The build produces single-column and double-column manuscript PDFs and the
supplementary PDF. Manuscript-facing text intentionally contains no version
history. Historical measurements remain associated with the deployed
implementation; corrected code is evaluated only by the explicitly scoped
mock-stub benchmark.

Before submission, follow `ZENODO_DEPOSIT_GUIDE.md`, publish the deposit,
and insert the issued DOI in the manuscript Data Availability statement and
submission metadata.
