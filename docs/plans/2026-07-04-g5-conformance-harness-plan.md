# G5 Conformance Harness v0.1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to
> implement this plan task-by-task after explicit implementation approval. Steps use checkbox
> (`- [ ]`) syntax for tracking.

**Goal:** Build a local GRCL Platform conformance harness that detects contract drift across docs,
schemas, C ABI headers, capability fixtures, MCU profile fixtures, and the existing M1 runnable
`grcl-c` core.

**Architecture:** G5 is a local contract-drift harness, not a CI, Docker, package-manager, or
release-infrastructure rollout. It should compose focused checks into one local runner while
preserving the existing artifact-root rule: generated outputs go under the workspace `artifacts/`
tree by default or under `GRCL_PLATFORM_ARTIFACT_ROOT` when that variable is set. The harness may
wrap existing M1 tests, but it must not add runtime behavior or change public ABI/schema semantics.

**Tech Stack:** POSIX shell for local orchestration, Python 3 standard library for portable static
checks and YAML-like fixture validation, the existing C compiler and C++ compiler used by
`src/grcl-c/tests/run_m1_tests.sh`, repository Markdown/YAML/C header artifacts.

---

## Scope

G5 includes only local conformance checks over artifacts that already exist after M1:

- documentation consistency and hygiene checks.
- runtime capability and negotiation fixture validation against existing schemas.
- MCU profile fixture validation against existing schema and architecture constraints.
- C ABI/header checks for public header hygiene and C11/C++17 compile coverage.
- integration of the existing M1 runnable harness as one conformance stage.
- one local top-level conformance command that can run from the workspace root and respect
  `GRCL_PLATFORM_ARTIFACT_ROOT`.

G5 does not include:

- GitHub Actions workflows, CI provider choices, Dockerfiles, containers, or package-manager policy.
- repo-wide build-system introduction.
- SDK wrapper implementation.
- publish/subscribe, service/client, endpoint matching, graph behavior, graph deltas, transport,
  executor scheduling, ROS2 adapter behavior, MCU runtime behavior, simulator runtime behavior, or
  management-plane implementation.
- authentication, remote management, event streams, release signing, registry, or external
  `github.com/alibenD/grcl` migration.

## Canonical Design Home

The canonical design home for G5 is
[Conformance And CI Strategy](../architecture/conformance-and-ci-strategy.md). G5 implementation
must update that document when it changes the conformance model. Long-lived design explanations
must not live only in this plan, status files, task briefs, implementation reports, or audit
reports.

## Allowed Files For G5 Implementation

Implementation batches may modify or create only these paths unless a task brief narrows the list
further:

- `scripts/check-docs.py`
- `scripts/check-schema-fixtures.py`
- `scripts/check-c-abi.py`
- `scripts/run-conformance.sh`
- `docs/architecture/conformance-and-ci-strategy.md`
- `docs/status/current-context.md`
- `docs/status/goal-execution-queue.md`
- `docs/status/middleware-goal-roadmap.md`
- `docs/README.md`
- `README.md`
- `schemas/*.yaml`
- `tests/conformance/runtime-capability/*.yaml`
- `tests/conformance/mcu-profiles/*.yaml`
- `src/grcl-c/tests/README.md`

The implementation must not edit these behavior surfaces without a new explicit decision:

- `src/grcl-c/include/grcl/c/*.h`
- `src/grcl-c/src/runtime.c`
- `src/grcl-runtime-native/src/null_backend.c`
- `src/grcl-c/tests/*_test.c`
- `src/grcl-c/tests/run_m1_tests.sh`

If a G5 check exposes a defect in a public header, runtime implementation, backend behavior, or M1
test, stop and report the defect instead of folding the fix into G5.

## Stop Conditions

Stop and ask the user for a decision if any G5 batch requires:

- a new public ABI, schema, protocol, lifecycle, endpoint, graph, transport, or executor semantic.
- a CI provider, Docker/container design, package-manager choice, or repo-wide build-system design.
- SDK behavior beyond verifying that SDKs must wrap `grcl-c`.
- runtime feature behavior beyond calling the existing M1 harness.
- schema or fixture changes that contradict accepted architecture documents rather than correcting a
  narrow validation issue.
- generated outputs under repository-root `build/`, `install/`, `log/`, caches, or generated report
  directories.
- write scope outside the allowed files above.

## Model And Effort Policy

| Work | Default model policy |
|---|---|
| G5 task-brief writing and closeout planning | GPT-5.5, high effort |
| Documentation check implementation | GPT-5.4, medium effort |
| Schema and fixture validation implementation | GPT-5.5, high effort if schema semantics are ambiguous; otherwise GPT-5.4, medium effort |
| C ABI/header conformance implementation | GPT-5.5, high effort |
| M1 harness integration | GPT-5.4, medium effort |
| Independent audit | GPT-5.5, high effort |

## Parallelization Plan

After G5 implementation is explicitly approved:

- `G5-B` documentation checks and `G5-C` runtime capability fixture validation may run in parallel
  because their write sets are disjoint.
- `G5-D` MCU profile validation and `G5-E` C ABI/header checks may run in parallel because their
  write sets are disjoint.
- `G5-F` top-level runner and M1 harness integration must wait until check command names, artifact
  paths, and exit-code conventions from `G5-B` through `G5-E` are known.
- `G5-G` closeout is sequential and requires an independent audit after all implementation batches
  are accepted.

## Task Queue

### G5-A: File G5 Plan And Durable Queue State

**Files:**

- Create: `docs/plans/2026-07-04-g5-conformance-harness-plan.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/README.md`
- Modify: `README.md`

**Steps:**

- [ ] Record G5 as a local conformance-harness planning goal.
- [ ] Add this plan to durable recovery and document navigation.
- [ ] Record that G5 implementation is not authorized until the user approves a G5 implementation
  window.
- [ ] Validate documentation hygiene with `git diff --check` and status consistency scans.
- [ ] Commit the planning changes separately from future implementation batches.

**Exit criteria:**

- G5 plan exists and is linked from durable recovery state.
- Status files agree that G5 planning is complete and G5 implementation is pending user approval.
- No runtime, backend behavior, SDK, CI, Docker, or build-system files are changed.

### G5-B: Documentation Conformance Checks

**Files:**

- Create: `scripts/check-docs.py`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`

**Required behavior:**

- Check Markdown links in `README.md`, `AGENTS.md`, and `docs/**/*.md`.
- Reject unresolved local links to missing files.
- Reject unfinished-marker terms in durable docs: `T[B]D`, `T[O]DO`, `fill[ ]in`,
  `implement[ ]later`, `place[ ]holder`.
- Verify that `docs/status/current-context.md`, `docs/status/goal-execution-queue.md`, and
  `docs/status/middleware-goal-roadmap.md` agree on the current milestone state.

**Verification commands:**

```bash
python3 scripts/check-docs.py
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-docs python3 scripts/check-docs.py
```

**Expected result:** both commands exit `0`, write generated diagnostics only under the configured
artifact root, and do not create repository-root generated output directories.

### G5-C: Runtime Capability Fixture Validation

**Files:**

- Create: `scripts/check-schema-fixtures.py`

**Required behavior:**

- Validate `tests/conformance/runtime-capability/accepted.yaml`,
  `tests/conformance/runtime-capability/degraded_accepted.yaml`, and
  `tests/conformance/runtime-capability/rejected_incompatible.yaml` against the required fields in
  `schemas/runtime-capability-record.schema.yaml` and
  `schemas/capability-negotiation-result.schema.yaml`.
- Check that accepted, degraded, and rejected fixtures use the expected negotiation status.
- Check that fixture values preserve capability, availability, and health separation.

**Verification commands:**

```bash
python3 scripts/check-schema-fixtures.py --runtime-capability
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-schema python3 scripts/check-schema-fixtures.py --runtime-capability
```

**Expected result:** both commands exit `0` and report the three runtime capability fixtures as
valid.

### G5-D: MCU Profile Fixture Validation

**Files:**

- Modify: `scripts/check-schema-fixtures.py`

**Required behavior:**

- Validate `tests/conformance/mcu-profiles/baremetal-min.yaml`,
  `tests/conformance/mcu-profiles/rtos-basic.yaml`, and
  `tests/conformance/mcu-profiles/rtos-posix-lite.yaml` against
  `schemas/mcu-profile.schema.yaml`.
- Check that MCU fixtures do not imply full distributed graph obligations.
- Check that bounded storage and heap-after-init claims remain explicit.

**Verification commands:**

```bash
python3 scripts/check-schema-fixtures.py --mcu-profiles
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-mcu python3 scripts/check-schema-fixtures.py --mcu-profiles
```

**Expected result:** both commands exit `0` and report the three MCU profile fixtures as valid.

### G5-E: C ABI/Header Conformance Checks

**Files:**

- Create: `scripts/check-c-abi.py`
- Modify: `src/grcl-c/tests/README.md`

**Required behavior:**

- Compile `src/grcl-c/tests/compile_headers_smoke.c` as C11.
- Compile `src/grcl-c/tests/compile_headers_smoke.cpp` as C++17.
- Reject public header references to C++ standard library types, exceptions, templates, RTTI,
  `rcl`, `rmw`, `rclcpp`, ROS2 identifiers, socket APIs, pthread APIs, Docker, CI, or build-system
  declarations.
- Check that public ABI structs that cross boundaries keep `struct_size` and `abi_version` where
  required by the architecture docs.

**Verification commands:**

```bash
python3 scripts/check-c-abi.py
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-cabi python3 scripts/check-c-abi.py
```

**Expected result:** both commands exit `0`, compile the C and C++ smoke tests, and write compiler
outputs only under the artifact root.

### G5-F: Top-Level Local Conformance Runner

**Files:**

- Create: `scripts/run-conformance.sh`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/README.md`
- Modify: `README.md`

**Required behavior:**

- Run documentation checks, runtime capability fixture validation, MCU profile validation, C ABI
  checks, and the existing `src/grcl-c/tests/run_m1_tests.sh`.
- Support execution from the workspace root.
- Support execution from the repository root when documented.
- Preserve `GRCL_PLATFORM_ARTIFACT_ROOT`.
- Use the workspace `artifacts/g5/conformance/` path by default.

**Verification commands:**

```bash
src/grcl-platform/scripts/run-conformance.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-conformance src/grcl-platform/scripts/run-conformance.sh
```

From repository root:

```bash
scripts/run-conformance.sh
```

**Expected result:** all three commands exit `0`, include the M1 harness in their output, and do not
write generated output into repository-root `build/`, `install/`, or `log`.

### G5-G: Closeout And Independent Audit

**Files:**

- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/README.md`
- Modify: `README.md`

**Required behavior:**

- Rerun all G5 conformance commands from the workspace root and repository root where applicable.
- Rerun `git diff --check`.
- Rerun forbidden-scope scans over `scripts/`, `src/grcl-c/`, and `src/grcl-runtime-native/`.
- Verify no repository-root generated output directories exist.
- Write implementation and audit reports under
  `.local/agentic-runs/2026-07-04-g5-conformance-harness/<task-id>/`.
- Update durable state only after independent audit acceptance.

**Verification commands:**

```bash
git -C src/grcl-platform diff --check
src/grcl-platform/scripts/run-conformance.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g5-closeout src/grcl-platform/scripts/run-conformance.sh
find src/grcl-platform -maxdepth 2 \( -name build -o -name install -o -name log \) -print
rg -n 'Dockerfile|\.github|colcon|ament|ros2|rclcpp|rmw_|rcl_|socket\(|pthread_|add_executable|project\(' src/grcl-platform/scripts src/grcl-platform/src/grcl-c src/grcl-platform/src/grcl-runtime-native
```

**Expected result:** conformance commands exit `0`; artifact scan exits `0` with no output;
forbidden-scope scan exits `1` with no output.

## Audit Checklist

Independent audit for each implementation batch must verify:

- the task stayed inside its allowed files and non-goals.
- generated outputs are under the workspace artifact root or an explicit `/tmp` override.
- no CI, Docker, build-system, SDK, pub/sub, graph, transport, ROS2, MCU runtime, simulator, or
  management-plane implementation was introduced.
- checks fail loudly on missing or malformed target artifacts rather than silently skipping them.
- documentation and status files remain internally consistent.
- all declared verification commands were run after the implementation changes.

## Planning Goal Closeout

This document completes planning only. G5 implementation requires a new explicit user approval
window before task briefs are created and implementation subagents are dispatched.
