# M5 Local Core Product Surface And GRCL-CPP Completion Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task after the design
> review gates pass. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver the first feature-complete local GRCL core product baseline by closing the native
backend `grcl-c` local-core surface, fully wrapping that surface in `grcl-cpp`, and reaching
passing core tests plus runnable C and C++ examples.

**Architecture:** M5 treats M4 as the hardened C baseline and extends it in one dependency-ordered
band: first close any remaining `grcl-c` local-core contract gaps that block stable wrappering,
then implement `grcl-cpp` as a faithful C++ ownership and usability layer over the same semantic
surface, then add C++ examples and local conformance. "Complete" in M5 means complete for the
single-process native in-process backend and the local core middleware product surface, not complete
for ROS2, networking, simulator, MCU, gateway, management plane, or release distribution.

**Tech Stack:** C11 runtime/tests/examples, C++17 wrapper/tests/examples, shell runners, existing
Python documentation and boundary checks, local artifact-root enforcement, implementation
subagents plus independent audit subagents.

---

## Scope

M5 includes:

- the native-backend local-core `grcl-c` surface as the semantic source of truth:
  runtime, node, publisher, subscription, service, client, executor, type support,
  runtime-local params, capability query and negotiation, and diagnostics retrieval.
- the minimum `grcl-c` API, contract, and runnable-behavior closure required so the local-core
  surface can be treated as a stable C baseline for wrappering and examples.
- a full `grcl-cpp` wrapper over the approved local-core C surface, with RAII ownership, typed or
  type-safe C++ facades where the mapping is direct, and no independent runtime semantics.
- local C++ tests and C++ examples that exercise the same core product surface already proven in C.
- local conformance expansion so the top-level runner can catch drift across docs, C ABI, C
  behavior, C++ wrappers, and C/C++ examples.

M5 excludes:

- ROS2, DDS, sockets, shared memory, networking, multi-process transport, simulator backend, MCU
  runtime, gateway runtime, management plane, auth, remote management, event streams, CI, Docker,
  package manager, CMake, colcon, release packaging, binary distribution, external `grcl`
  migration, and `grcl-py` expansion beyond the completed G6 boundary skeleton.
- new product domains outside the local core surface, including graph cache APIs, distributed
  params, node-scoped params, wait sets, futures, blocking calls, background threads, zero-copy,
  IDL/codegen, serialization design, or transport sessions.
- C++ APIs whose behavior cannot be explained as a direct wrapper over the underlying `grcl-c`
  contract.

## Definition Of Complete

M5 is complete only when all of the following are true:

- `grcl-c` exposes a reviewed and test-backed local-core surface for runtime, node, publisher,
  subscription, service, client, executor, type support, runtime-local params, capability, and
  diagnostics on the native in-process backend.
- the native in-process backend can run the approved local core scenarios for those components
  without introducing background threads, distributed transport, or ROS2 behavior.
- `grcl-cpp` wraps the same local-core surface with no independent lifecycle, ownership, backend,
  capability, or executor semantics.
- local core tests pass for both `grcl-c` and `grcl-cpp`.
- runnable C and C++ examples compile and pass from repository root, workspace root, and
  artifact-root override.

M5 completion does **not** mean the whole GRCL platform is complete. It does not cover ROS2, MCU,
simulator, gateway, management plane, remote control, cross-process transport, or release
distribution.

## M5 Principles

- `grcl-c` remains the semantic source of truth. `grcl-cpp` may improve ergonomics, but it must not
  invent behavior.
- Narrow `grcl-c` contract changes are allowed only when design review plus failing tests prove the
  current M4 baseline is insufficient for the approved local-core product surface.
- `grcl-cpp` wrappers must preserve backend privacy. No backend-private structs, transport objects,
  or ROS2 types may appear in public C++ headers.
- Examples are acceptance evidence, not the only evidence. Every new user-visible core feature path
  must be backed by targeted tests first.
- Local runners must continue to respect `GRCL_PLATFORM_ARTIFACT_ROOT` and must not rely on a
  repo-wide build-system rollout.

## Stop Conditions

- local-core completion requires a new public schema, protocol, or ABI policy that cannot be
  decided from the existing M3/M4 baseline and current design homes.
- `grcl-cpp` wrapper design requires choosing a package/build/distribution strategy rather than a
  direct local wrapper shape.
- the next task would expand into ROS2, transport, threading, serialization, `grcl-py` expansion,
  management plane, or release engineering.
- an audit rejects the design or implementation and the issue cannot be fixed by a narrow in-scope
  corrective task.

## Task Queue

### M5-A: File Plan And Durable Queue State

Files:

- Create: `docs/plans/2026-07-05-m5-local-core-product-and-grcl-cpp-completion-plan.md`
- Modify: `README.md`
- Modify: `docs/README.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `docs/architecture/system-overview.md`
- Modify: `docs/architecture/language-sdk-strategy.md`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`

Steps:

- [ ] Record M5 as the active post-M4 user-approved goal.
- [ ] Normalize the user's broad end-state request into the local-core product definition above.
- [ ] Link this plan from repository navigation, status, and canonical design homes.
- [ ] Add dependency-ordered queue items for design review, implementation batches, conformance,
  and closeout.
- [ ] Remove stale status language that still describes M3 or pre-M5 state as active.

Exit criteria:

- M5 is discoverable from `README.md`, `docs/README.md`, current context, goal queue, roadmap, and
  the relevant canonical design homes.
- No runtime, backend, SDK, test, or example implementation source is changed in M5-A.

### M5-B: Design Closure And Review Baseline

Files:

- Modify: `docs/architecture/grcl-c-api-shape.md`
- Modify: `docs/architecture/backend-spi-contract.md`
- Modify: `docs/architecture/language-sdk-strategy.md`
- Modify: `docs/architecture/system-overview.md`
- Create or update: `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-b-design-review/`

Steps:

- [ ] Close the local-core component inventory and identify any remaining `grcl-c` contract gaps
  that block a stable native-backend product baseline.
- [ ] Define the `grcl-cpp` wrapper surface for runtime, node, executor, publisher, subscription,
  service, client, and runtime-local params.
- [ ] Define the local acceptance matrix for C tests, C++ tests, C examples, C++ examples, and
  top-level conformance integration.
- [ ] Dispatch architecture/API, C++ wrapper, and test-plan reviews.
- [ ] If any review rejects the design, update the canonical design homes before implementation.

Exit criteria:

- The local-core completion target is explicit in the canonical design homes.
- Design review returns `accepted` or `accepted_with_notes`.
- No code implementation starts before the review gate passes.

### M5-C: GRCL-C Local-Core Surface Closeout

Files:

- Modify only as required by approved design plus failing tests:
  `src/grcl-c/include/grcl/c/runtime.h`
- Modify only as required by approved design plus failing tests:
  `src/grcl-c/src/runtime.c`
- Modify only as required by approved design plus failing tests:
  `src/grcl-runtime-native/src/inprocess_backend.c`
- Create or modify: `src/grcl-c/tests/*.c`
- Create or modify: `src/grcl-c/tests/run_m5_c_tests.sh`
- Create or modify: `examples/c/*.c`
- Create or modify: `examples/c/run_m5_c_examples.sh`

Steps:

- [ ] Write or extend failing C tests for any approved M5 `grcl-c` local-core contract gaps.
- [ ] Run the targeted C runners and record RED.
- [ ] Implement the minimum `grcl-c` and native-backend changes required by the failing tests.
- [ ] Re-run targeted C runners, the M4 contract runner, the M3 example runner, and top-level
  conformance; record GREEN.
- [ ] Update canonical `grcl-c` design documents only for the approved narrow contract changes.

Exit criteria:

- The approved local-core C surface is complete and test-backed.
- No out-of-scope transport, serialization, graph, ROS2, or management behavior is introduced.

### M5-D: GRCL-CPP Runtime, Node, And Executor Wrappers

Files:

- Modify: `src/grcl-cpp/include/grcl/cpp/runtime.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/node.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/executor.hpp`
- Create or modify: `src/grcl-cpp/tests/runtime_node_executor_test.cpp`
- Create or modify: `src/grcl-cpp/tests/run_m5_cpp_tests.sh`

Steps:

- [ ] Write failing C++ tests for runtime ownership, node ownership, executor membership, move-only
  semantics, and forwarding of `grcl-c` result behavior.
- [ ] Run the targeted C++ runner and record RED.
- [ ] Implement the minimum wrapper code to make the tests pass without inventing new semantics.
- [ ] Re-run the targeted C++ runner plus relevant C and conformance runners; record GREEN.
- [ ] Audit for direct `grcl-c` mapping and backend privacy.

Exit criteria:

- Runtime, node, and executor wrappers are usable from C++ while remaining semantically equivalent
  to `grcl-c`.
- Wrapper code remains free of backend-private, ROS2, socket, or threading leakage.

### M5-E: GRCL-CPP Pub/Sub Wrappers And Examples

Files:

- Create: `src/grcl-cpp/include/grcl/cpp/publisher.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/subscription.hpp`
- Create or modify: `src/grcl-cpp/tests/pub_sub_test.cpp`
- Create or modify: `src/grcl-cpp/tests/run_m5_cpp_tests.sh`
- Create: `examples/cpp/pub_sub_example.cpp`

Steps:

- [ ] Write failing C++ tests for publisher/subscription lifecycle, publish-bytes forwarding,
  subscription take semantics, and `spin_once`-driven delivery.
- [ ] Run the targeted C++ runner and record RED.
- [ ] Implement the minimum wrappers required by the tests.
- [ ] Add a runnable C++ pub/sub example and verify it through the approved local runner.
- [ ] Re-run the targeted C++ runner plus relevant C and conformance runners; record GREEN.

Exit criteria:

- C++ pub/sub wraps the approved `grcl-c` local-core behavior without adding transport or
  background-thread semantics.
- The C++ pub/sub example runs locally against the native in-process backend.

### M5-F: GRCL-CPP Service, Client, And Runtime-Local Params

Files:

- Create: `src/grcl-cpp/include/grcl/cpp/service.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/client.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/params.hpp`
- Create or modify: `src/grcl-cpp/tests/service_client_test.cpp`
- Create or modify: `src/grcl-cpp/tests/params_test.cpp`
- Create or modify: `src/grcl-cpp/tests/run_m5_cpp_tests.sh`
- Create: `examples/cpp/service_client_example.cpp`
- Create: `examples/cpp/params_example.cpp`

Steps:

- [ ] Write failing C++ tests for service/client request-reply forwarding, correlation behavior,
  no-blocking-call rules, and runtime-local param set/get/list semantics.
- [ ] Run the targeted C++ runner and record RED.
- [ ] Implement the minimum wrappers required by the tests.
- [ ] Add runnable C++ service/client and params examples.
- [ ] Re-run the targeted C++ runner plus relevant C and conformance runners; record GREEN.

Exit criteria:

- Service/client and runtime-local params are wrapped in C++ with no independent protocol,
  scheduling, or distributed-param semantics.
- The C++ service/client and params examples run locally against the native in-process backend.

### M5-G: C++ Example Matrix And Conformance Integration

Files:

- Create: `examples/cpp/core_middleware_example.cpp`
- Create: `examples/cpp/run_m5_cpp_examples.sh`
- Modify: `scripts/run-conformance.sh`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `README.md`
- Modify: `docs/README.md`

Steps:

- [ ] Add a combined C++ local-core example that exercises runtime, node, executor, pub/sub,
  service/client, and runtime-local params.
- [ ] Add a dedicated C++ example runner.
- [ ] Integrate the approved C++ wrapper/example runner into the top-level local conformance flow.
- [ ] Run conformance from repository root, workspace root, and artifact-root override.
- [ ] Run artifact-pollution scan.

Exit criteria:

- Top-level local conformance catches drift across docs, C ABI, C behavior, C++ wrappers, and C++
  examples.
- Generated artifacts stay under `GRCL_PLATFORM_ARTIFACT_ROOT` or workspace `artifacts/`.

### M5-H: Closeout Verification And Durable Status Update

Files:

- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `README.md`
- Modify: `docs/README.md`

Steps:

- [ ] Run final M5 verification: top-level conformance from repository root, workspace root, and
  artifact-root override; targeted C runner; targeted C++ runner; C example runner; C++ example
  runner; docs check; SDK boundary drift check; artifact-pollution scan.
- [ ] Dispatch independent closeout audit.
- [ ] After audit acceptance only, mark M5 complete in durable status documents.
- [ ] Commit the closeout as a task-bounded commit sequence.

Exit criteria:

- M5 closeout audit returns `accepted` or `accepted_with_notes` with no blocking findings.
- Durable status marks M5 complete only after closeout audit acceptance.
- No post-M5 implementation milestone is authorized from M5 completion alone.

## Required Verification Commands

Planning and documentation-only tasks:

```sh
git diff --check
python3 scripts/check-docs.py
scripts/run-conformance.sh
```

Implementation batches will extend verification with targeted C runners, targeted C++ runners, and
example runners as those artifacts are introduced.
