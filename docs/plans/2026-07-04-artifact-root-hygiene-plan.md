# Artifact Root Hygiene Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prevent generated artifacts from drifting into non-canonical workspace paths.

**Architecture:** Keep `/Users/aliben/Project/grcl-platform_ws/artifacts` as the only default local
artifact root. Fix the G6 Python runner to resolve that root consistently, and add a conformance
hygiene gate that fails if a stale `src/artifacts` tree appears.

**Tech Stack:** POSIX shell test runners, local conformance runner, workspace-level generated
artifact root.

---

## Scope

This plan is a narrow process-hygiene batch. It does not change runtime behavior, SDK semantics,
backend code, or M3 implementation scope.

Allowed files:

- `scripts/run-conformance.sh`
- `scripts/README.md`
- `src/grcl-py/tests/run_g6_py_tests.sh`
- `docs/plans/2026-07-04-artifact-root-hygiene-plan.md`
- `.local/agentic-runs/2026-07-04-artifact-root-hygiene/`

Allowed generated-state cleanup:

- Remove empty `/Users/aliben/Project/grcl-platform_ws/src/artifacts` after the runner default is
  fixed.

## Task 1: Fix G6 Python Artifact Root

Files:

- Modify: `src/grcl-py/tests/run_g6_py_tests.sh`

Steps:

- [ ] Verify the existing default resolves from repository root to `../artifacts`, which is
      `/Users/aliben/Project/grcl-platform_ws/src/artifacts` in the standard workspace layout.
- [ ] Change the default to derive `workspace_root` from `repo_root/../..`, matching the G6 C++
      runner pattern.
- [ ] Keep `GRCL_PLATFORM_ARTIFACT_ROOT` as the explicit override path.
- [ ] Run the G6 Python test runner without `GRCL_PLATFORM_ARTIFACT_ROOT` and verify output is
      written under `/Users/aliben/Project/grcl-platform_ws/artifacts/g6/grcl-py`.

## Task 2: Add Artifact Root Hygiene Gate

Files:

- Modify: `scripts/run-conformance.sh`
- Modify: `scripts/README.md`

Steps:

- [ ] Add a first conformance stage that checks the forbidden
      `/Users/aliben/Project/grcl-platform_ws/src/artifacts` path does not exist.
- [ ] Make the failure message explain that generated artifacts must use the workspace-level
      `artifacts/` directory or `GRCL_PLATFORM_ARTIFACT_ROOT`.
- [ ] Update the stage count and summary so conformance output records the hygiene gate.
- [ ] Document in `scripts/README.md` that scripts must not create `src/artifacts`.

## Task 3: Cleanup, Audit, And Closeout

Files:

- Add: `.local/agentic-runs/2026-07-04-artifact-root-hygiene/artifact-root-hygiene/task-brief.md`
- Add: `.local/agentic-runs/2026-07-04-artifact-root-hygiene/artifact-root-hygiene/implementation-report.md`
- Add: `.local/agentic-runs/2026-07-04-artifact-root-hygiene/artifact-root-hygiene/audit-report.md`
- Add: `.local/agentic-runs/2026-07-04-artifact-root-hygiene/artifact-root-hygiene/ledger.md`

Steps:

- [ ] Remove the empty generated `/Users/aliben/Project/grcl-platform_ws/src/artifacts` tree only
      after the script default is fixed.
- [ ] Record implementation evidence in `implementation-report.md`.
- [ ] Run an independent audit against this plan, the changed files, and verification evidence.
- [ ] Run final verification from the workspace root.
- [ ] Commit only this plan's tracked source/doc changes, leaving unrelated M3 work untouched.
