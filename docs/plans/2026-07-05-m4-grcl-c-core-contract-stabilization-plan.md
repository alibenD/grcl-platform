# M4 GRCL-C Core Contract Stabilization And Conformance Expansion Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task after the design
> review gates pass. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stabilize the M3 `grcl-c` core contract by converting the runnable local middleware
baseline into stronger negative-path, ownership, capacity, and conformance evidence without
expanding into new middleware features.

**Architecture:** M4 treats M3 as the first runnable local core baseline and hardens it rather than
adding transport, graph, SDK, or management behavior. The public `grcl-c` handles remain
core-owned, the native backend remains backend-private and in-process, and conformance remains
local. Any production-code change in M4 must be driven by a failing contract test that exposes drift
between the documented M3 contract and the current implementation.

**Tech Stack:** C11 tests and examples, C++17 header smoke compatibility, shell runners, existing
Python conformance scripts, local artifact root, implementation subagents plus independent audit
subagents.

---

## Scope

M4 includes:

- `grcl-c` core contract stabilization for the M3 surface:
  runtime, node, publisher, subscription, service, client, executor, type support, runtime-local
  params, and native in-process backend behavior.
- Negative-path and boundary conformance for ownership, lifecycle, destroyed handles, wrong-runtime
  usage, missing backend hooks, type mismatch, no-data, capacity, payload size, and deterministic
  dispatch.
- A dedicated M4 local contract test runner, integrated into the top-level local conformance runner
  after the M3 examples stage.
- Durable documentation updates in the existing canonical design homes; no new parallel design
  index.

M4 excludes:

- New public feature areas beyond the M3 `grcl-c` local core contract.
- ROS2, DDS, sockets, shared memory, networking, multi-process transport, simulator backend, MCU
  runtime, gateway, management plane, auth, remote management, event streams, CI, Docker, package
  manager, CMake, colcon, IDL/codegen, external `grcl` migration, C++/Python example acceptance,
  and release-stability claims.
- Blocking waits, future handles, background threads, zero-copy transport, distributed params,
  node-scoped params, graph cache behavior, and SDK usability expansion.

## Stabilization Principles

- M4 may strengthen tests before changing production code; production code changes require RED
  evidence from a specific contract test.
- M4 must not treat examples as the only proof of correctness. Examples remain smoke acceptance;
  contract tests must cover edge cases and failure modes.
- M4 must preserve append-only ABI discipline. Public header changes are allowed only if a design
  review and failing header/contract test prove that the current M3 contract cannot express the
  documented behavior.
- M4 must keep native backend internals private. Tests may observe public results and behavior, not
  backend-private structs.
- M4 must keep `null/native-test` as a lifecycle/object-lifecycle baseline and must not make it
  claim native in-process routing, params, or messaging capability.

## Task Queue

### M4-A: File Plan And Durable Queue State

Files:

- Create: `docs/plans/2026-07-05-m4-grcl-c-core-contract-stabilization-plan.md`
- Modify: `README.md`
- Modify: `docs/README.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`

Steps:

- [ ] Record M4 as the active user-approved post-M3 goal.
- [ ] Link this plan from repository navigation and durable recovery state.
- [ ] Add dependency-ordered queue items for design review, implementation batches, conformance
  integration, and closeout.
- [ ] Validate documentation hygiene.

Exit criteria:

- M4 is discoverable from `README.md`, `docs/README.md`, current context, goal queue, and roadmap.
- No runtime source, backend source, public header, test, example, or conformance runner source is
  changed in M4-A.

### M4-B: Independent Contract Stabilization Design Reviews

Files:

- `.local/agentic-runs/2026-07-05-m4-grcl-c-core-contract-stabilization/m4-b-api-review/`
- `.local/agentic-runs/2026-07-05-m4-grcl-c-core-contract-stabilization/m4-b-backend-review/`
- `.local/agentic-runs/2026-07-05-m4-grcl-c-core-contract-stabilization/m4-b-test-plan-review/`

Steps:

- [ ] Dispatch API contract review for handle ownership, lifecycle states, destroyed-handle
  behavior, cross-runtime usage, result-code consistency, and ABI compatibility.
- [ ] Dispatch backend containment review for native in-process privacy, `null/native-test`
  unsupported behavior, append-only SPI use, and forbidden scope.
- [ ] Dispatch test-plan review for coverage of negative paths, capacity limits, deterministic
  dispatch, examples-versus-contract separation, and conformance integration.
- [ ] If any review rejects the plan, update this plan or canonical design homes before
  implementation.

Exit criteria:

- API, backend, and test-plan reviews return `accepted` or `accepted_with_notes`.
- Any rejection produces a narrow design-fix task before implementation begins.

### M4-C: Core Lifecycle And Ownership Contract Tests

Files:

- Create: `src/grcl-c/tests/core_lifecycle_contract_test.c`
- Create or modify: `src/grcl-c/tests/run_m4_contract_tests.sh`
- Modify only if failing tests prove a contract bug: `src/grcl-c/src/runtime.c`

Steps:

- [ ] Write failing tests for null required pointers, invalid option struct sizes, wrong lifecycle
  state, destroyed runtime usage, destroyed node usage, and deterministic runtime destroy cleanup.
- [ ] Write failing tests for cross-runtime object misuse: adding a node to an executor from another
  runtime, creating endpoints with destroyed nodes, and destroying objects in non-creation order.
- [ ] Run the M4 contract runner and record RED.
- [ ] Implement the minimum `runtime.c` fixes required by the failing tests.
- [ ] Re-run the M4 contract runner and M1 harness; record GREEN.

Exit criteria:

- Lifecycle and ownership contract tests pass.
- Existing M1 harness and M3 examples remain green.
- No public header or backend behavior expansion is introduced unless separately approved by design
  review.

### M4-D: Messaging And Executor Contract Tests

Files:

- Create: `src/grcl-c/tests/core_messaging_contract_test.c`
- Modify: `src/grcl-c/tests/run_m4_contract_tests.sh`
- Modify only if failing tests prove a contract bug: `src/grcl-c/src/runtime.c`
- Modify only if failing tests prove a native routing bug:
  `src/grcl-runtime-native/src/inprocess_backend.c`

Steps:

- [ ] Write failing tests for no delivery before `spin_once`, one bounded dispatch pass per
  `spin_once`, FIFO delivery, topic and `type_id` mismatch, payload-too-large, queue saturation,
  and small output buffers.
- [ ] Write failing tests for service/client request id correlation, unknown request id,
  no matching service, request/response queue saturation, type mismatch, and deterministic
  spin-driven routing.
- [ ] Run the M4 contract runner and record RED.
- [ ] Implement the minimum core/native fixes required by the failing tests.
- [ ] Re-run the M4 contract runner, M1 harness, and M3 examples; record GREEN.

Exit criteria:

- Messaging and executor contract tests pass.
- `spin_once` remains non-blocking and does not create background threads.
- No transport, socket, ROS2, simulator, graph, or SDK behavior is introduced.

### M4-E: Runtime Params And Capability Contract Tests

Files:

- Create: `src/grcl-c/tests/core_params_capability_contract_test.c`
- Modify: `src/grcl-c/tests/run_m4_contract_tests.sh`
- Modify only if failing tests prove a contract bug: `src/grcl-c/src/runtime.c`
- Modify only if failing tests prove a native params or capability bug:
  `src/grcl-runtime-native/src/inprocess_backend.c`
- Modify only if failing tests prove a null/native-test capability bug:
  `src/grcl-runtime-native/src/null_backend.c`

Steps:

- [ ] Write failing tests for zero-size parameter values, invalid names, unknown types,
  overwrite semantics, list buffer boundaries, table saturation, name-storage saturation, and
  value-storage saturation.
- [ ] Write failing tests that native in-process capability reports only implemented M3 local
  behavior and that `null/native-test` does not claim runtime-local params, routing, or executor
  dispatch.
- [ ] Run the M4 contract runner and record RED.
- [ ] Implement the minimum fixes required by the failing tests.
- [ ] Re-run the M4 contract runner, M1 harness, and top-level conformance runner; record GREEN.

Exit criteria:

- Runtime params and capability contract tests pass.
- Capability, availability, and health remain separate.
- No distributed params, node-scoped params, callbacks, watchers, descriptors, auth, management, or
  event-stream behavior is introduced.

### M4-F: Local Conformance Integration

Files:

- Modify: `scripts/run-conformance.sh`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/README.md`
- Modify: `README.md`

Steps:

- [ ] Add the M4 core contract test runner as a local conformance stage after the M3 examples
  stage.
- [ ] Update the conformance stage count and runner summary.
- [ ] Update conformance documentation to describe M4 as local contract hardening, not CI or
  release readiness.
- [ ] Run full conformance from repository root, workspace root, and artifact-root override.
- [ ] Run artifact-pollution scan.

Exit criteria:

- Top-level local conformance passes with the M4 stage included.
- Documentation accurately describes runner stages and the local-only M4 boundary.
- Generated artifacts stay under `GRCL_PLATFORM_ARTIFACT_ROOT` or workspace `artifacts/`.

### M4-G: Closeout Verification And Durable Status Update

Files:

- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `README.md`
- Modify: `docs/README.md`

Steps:

- [ ] Run final M4 verification: top-level conformance from repository root, workspace root, and
  artifact-root override; M4 runner; M3 examples; M1 harness; docs check; SDK boundary drift check;
  artifact-pollution scan.
- [ ] Dispatch independent closeout audit.
- [ ] After audit acceptance only, mark M4 complete in durable status documents.
- [ ] Commit the closeout as a task-bounded commit.

Exit criteria:

- M4 closeout audit returns `accepted` or `accepted_with_notes` with no blocking findings.
- Durable status marks M4 complete only after closeout audit acceptance.
- No post-M4 implementation milestone is authorized from M4 completion alone.

## Required Verification Commands

Planning and documentation-only tasks:

```sh
git diff --check
python3 scripts/check-docs.py
```

Implementation tasks:

```sh
src/grcl-c/tests/run_m4_contract_tests.sh
src/grcl-c/tests/run_m1_tests.sh
examples/c/run_m3_examples.sh
scripts/run-conformance.sh
git diff --check
python3 scripts/check-docs.py
python3 scripts/check-sdk-boundaries.py
find . -path './.git' -prune -o -type d \( -name build -o -name install -o -name log -o -name __pycache__ -o -name .venv -o -name dist -o -name '*.egg-info' \) -print
```

Closeout tasks must also verify:

```sh
src/grcl-platform/scripts/run-conformance.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-m4-conformance scripts/run-conformance.sh
```

## Stop Conditions

Stop and report before implementation if:

- M4 tests require public ABI/header changes that are not covered by the M4 design reviews.
- Stabilization requires new feature behavior rather than a fix to documented M3 contract behavior.
- Any task pulls in ROS2, networking, simulator, MCU runtime, management plane, auth, remote
  management, event streams, CI, Docker, package/build-system rollout, IDL/codegen, C++/Python
  example acceptance, or external `grcl` migration.
- The current M3 examples or top-level conformance runner cannot pass before M4 changes begin.
