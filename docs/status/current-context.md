# Current Context

## Purpose

This file is the durable recovery point for new Codex sessions working on GRCL Platform. Do not
rely on previous chat history as the source of truth. If this file conflicts with the repository
documents, inspect the referenced documents and ask for confirmation before editing.

## Workspace And Repository

Recommended Codex working directory:

```text
/Users/aliben/Project/grcl-platform_ws
```

Repository root:

```text
/Users/aliben/Project/grcl-platform_ws/src/grcl-platform
```

Remote:

```text
git@github.com:alibenD/grcl-platform.git
```

The workspace root is intentionally above the Git repository because Docker development, future
multi-repository orchestration, and generated artifacts need a stable workspace-level mount point.

## Current Development Mode

- M1 implementation is complete under the user-approved Autonomous Goal Runner window.
- Runtime/product code implementation is re-opened only inside the user-approved M5 scope.
- Autonomous Goal Runner mode was authorized for M1 implementation-stage task briefs,
  implementation subagents, and independent audit subagents.
- G1 v0.1 Contract Artifacts is complete.
- G1-A through G1-E passed independent audit.
- G2 `grcl-c` ABI Contract v0.1 is complete. G2-A resolved the capability C ABI representation
  gate as the recommended hybrid and passed independent audit. G2-B header hardening also passed
  independent audit. G2-C runtime lifecycle and endpoint declarations also passed independent
  audit. G2-D capability query and negotiation ABI shaping passed independent audit with
  non-blocking notes. G2-E compile-only smoke and conformance artifacts also passed independent
  audit with non-blocking notes. G2-F closeout is recorded in the canonical `grcl-c` API shape
  document.
- Middleware documentation information architecture cleanup is complete: `docs/README.md` is the
  global navigation entrypoint and `system-overview.md` is the whole-middleware canonical design
  home.
- G2-F `grcl-c` ABI v0.1 closeout is complete in
  [GRCL-C API Shape](../architecture/grcl-c-api-shape.md).
- G3 Backend SPI API Shape v0.1 design baseline is complete in
  [Backend SPI Contract](../architecture/backend-spi-contract.md). It defines SPI ownership,
  operation-table shape, lifecycle and capability hooks, and the minimum native/null backend test
  target that M1 has now materialized.
- M1 First Runnable GRCL-C Core With Null/Native Backend is complete against
  [M1 First Runnable GRCL-C Core Plan](../plans/2026-07-04-m1-first-runnable-core-plan.md). M1
  produced a first runnable `grcl-c` lifecycle, bounded-storage, capability negotiation, and
  diagnostics path through a private null/native-test backend.
- M1-B through M1-F passed independent audit. M1-G full local closeout verification passed from the
  workspace root, with `GRCL_PLATFORM_ARTIFACT_ROOT` override, and from the repository root. M1-G
  passed independent audit with notes under
  `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-g-closeout/`.
- G5 Conformance Harness v0.1 planning is complete in
  [G5 Conformance Harness Plan](../plans/2026-07-04-g5-conformance-harness-plan.md). G5 is scoped
  as a local contract-drift harness for docs, schemas, C ABI, capability fixtures, MCU profile
  fixtures, and M1 harness integration.
- G5 Conformance Harness v0.1 is complete under the user-approved goal window. G5-B through G5-G
  used task briefs, TDD-style verification, implementation subagents, and independent audit
  subagents. G5 provides the local `scripts/run-conformance.sh` contract-drift harness covering
  documentation checks, runtime capability fixtures, MCU profile fixtures, C ABI/header checks, and
  the M1 runnable harness.
- G6 Language SDK Wrapper Skeletons planning is complete in
  [G6 Language SDK Wrapper Skeletons Plan](../plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md).
  It records M2 Cross-Language SDK Boundary Baseline, future task breakdown, allowed file sets,
  verification strategy, and the Python binding substrate gate. The user selected Option A:
  `ctypes` or private dynamic-library shim boundary with package distribution deferred.
- G6 Language SDK Wrapper Skeletons implementation is complete under the user-approved goal-driven
  window. G6-C `grcl-cpp` passed independent audit with notes, G6-D `grcl-py` passed independent
  audit, and G6-E SDK boundary drift checks passed independent audit. The local conformance runner
  now includes documentation, schema fixture, C ABI/header, M1 harness, and SDK boundary drift
  stages.
- M3 Core Middleware Contract And Native In-Process Backend is complete under a user-approved plan:
  [M3 Core Middleware Native Backend Plan](../plans/2026-07-04-m3-core-middleware-native-backend-plan.md).
  M3 is limited to `grcl-c` core middleware contracts, backend SPI v0.2 design, an in-process
  native backend, TDD implementation batches, C examples, and local conformance integration.
  Design review has passed after architecture review `accepted_with_notes`, ABI/SPI re-review
  `accepted_with_notes`, and test-plan re-review `accepted`. M3-C API/SPI header contract is
  complete after independent audit `accepted_with_notes`. M3-D core object ownership is complete
  after independent audit `accepted_with_notes`. M3-E pub/sub routing is complete after re-audit
  `accepted_with_notes`. M3-F service/client routing is complete after independent audit
  `accepted_with_notes`. M3-G local runtime params is complete after independent audit `accepted`.
  M3-H native backend capability update is complete after independent audit `accepted`. M3-I C
  examples is complete after re-audit `accepted`. M3-J conformance integration and closeout is
  complete after independent closeout audit `accepted`; the local conformance runner now includes
  the M3 core middleware examples stage. M3-D audit recorded a non-blocking `runtime.c`
  create-failure cleanup hardening note; M3-F implemented a narrow cleanup hardening change when
  that path became testable.
- post-M3 implementation remains unauthorized unless a new user-approved goal window records a
  goal-specific plan, allowed files, verification strategy, implementation subagents, and
  independent audit gates.
- M4 GRCL-C Core Contract Stabilization And Conformance Expansion is complete under the user's
  approved goal request. M4 stabilized the completed M3 `grcl-c` local core contract
  through design review, negative-path contract tests, conformance expansion, implementation
  subagents, and independent audits. M4 does not authorize new public feature areas, ROS2,
  networking, simulator, MCU runtime, management plane, auth, remote management, event streams, CI,
  Docker, package/build-system rollout, IDL/codegen, C++/Python example acceptance, or external
  `grcl` migration.
- M4-B design review is complete: API re-review accepted after narrowing destroyed-runtime handle
  semantics; backend containment and test-plan reviews accepted with notes. M4-C core lifecycle and
  ownership contract tests are complete after independent audit acceptance. M4-D messaging and
  executor contract tests are complete after independent audit acceptance with a non-blocking
  response-type-mismatch coverage note. M4-E runtime params and capability contract tests are
  complete after independent audit acceptance. M4-F local conformance integration is complete after
  independent audit acceptance. M4-G closeout verification is complete after closeout audit
  acceptance with notes. No post-M4 implementation milestone is authorized without a new
  user-approved goal.
- The user has now approved M5 Local Core Product Surface And GRCL-CPP Completion as the next
  active implementation milestone. M5 normalizes the user's broad end-state request into one
  concrete target: complete the native-backend local-core product surface in `grcl-c`, fully wrap
  that surface in `grcl-cpp`, and verify it with passing core tests plus runnable C and C++
  examples.
- M5-A plan and durable queue filing is complete. M5-B design closure is also complete after
  independent API review `accepted_with_notes`, independent C++ wrapper review
  `accepted_with_notes`, an initial test-plan review `rejected`, a narrow design fix, and
  independent test-plan re-review `accepted`. The accepted M5-B design now makes the local-core
  completion target explicit, keeps endpoint exposure as opaque identity escape hatches only, keeps
  runtime-local params complete at `set/get/list`, and assigns capability/negotiation/diagnostics
  forwarding evidence to the M5-D C++ runtime-wrapper batch.
- M5-C `grcl-c` local-core surface closeout is complete after implementation report and independent
  audit `accepted_with_notes`. M5-C required no `grcl-c`, native-backend, C test, C example, or
  canonical design source changes: the accepted M5-B C-side completion boundary was already
  satisfied by current repository evidence and the full local verification ladder
  (`run_m1_tests.sh`, `run_m4_contract_tests.sh`, `run_m3_examples.sh`,
  `scripts/run-conformance.sh`) passed green from the workspace root.
- M5-D `grcl-cpp` runtime, node, and executor wrappers is complete after implementation report and
  independent audit `accepted_with_notes`. M5-D added move-only `Node` and `Executor` wrappers,
  extended `Runtime` with capability query, capability negotiation, and diagnostics forwarding,
  added a dedicated `run_m5_cpp_tests.sh` runner plus `runtime_node_executor_test.cpp`, and updated
  the existing G6 smoke runner to link the native in-process backend required by the current core
  runtime selection path. M5-D also applied one narrow `grcl-c` compile-hygiene unblock in
  `src/grcl-c/src/runtime.c` by removing an unused static helper that failed under the accepted
  `-Werror` runner policy.
- M5-E `grcl-cpp` pub/sub wrappers and example is complete after implementation report and
  independent audit `accepted_with_notes`. M5-E added move-only `Publisher` and `Subscription`
  wrappers, extended the dedicated M5 C++ runner with a focused pub/sub test, added the first
  runnable C++ pub/sub example plus example runner, and closed a lifecycle-order bug by making the
  C++ test and example explicitly destroy child wrappers before `Runtime::destroy()` under the
  current direct-wrapper ownership model.
- M5-F1 `grcl-cpp` service/client wrappers is complete as the first internal subtask under GQ-091
  after implementation report and independent audit `accepted_with_notes`. M5-F1 added move-only
  `Service` and `Client` wrappers, extended the dedicated M5 C++ runner with a focused
  service/client test, added the first runnable C++ service/client example plus runner coverage,
  and preserved direct `grcl-c` request/reply semantics with explicit `grcl_request_id_t`
  correlation and executor-driven delivery.
- M5-F2 `grcl-cpp` runtime-local params wrappers is now the next authorized internal subtask under
  GQ-091. M5 implementation batches must remain inside the approved local-core scope and continue
  to use task briefs, implementation subagents, TDD-style verification, and independent audit
  subagents.
- M5 does not authorize ROS2, DDS, sockets, shared memory, networking, multi-process transport,
  simulator backend, MCU runtime, gateway runtime, management plane, auth, remote management,
  event streams, CI, Docker, package manager, CMake, colcon, release packaging, external `grcl`
  migration, IDL/codegen, or `grcl-py` expansion beyond the completed G6 boundary skeleton.
- Management-plane concept modeling is deferred as future G10 input unless explicitly reprioritized.
- Runtime capability exchange is a design decision, not an implemented runtime feature.
- Active goal execution is tracked in `docs/status/goal-execution-queue.md`.
- Architecture design, ADR writing, and task planning should use GPT-5.5 high effort when
  available.
- Future implementation tasks should use implementation subagents plus independent audit subagents
  according to the agentic delivery governance document.
- Repository-local workflow classification, fast-path limits, TDD policy, task-bounded commits, and
  durable recovery updates are governed by
  [Task Workflow Governance](../architecture/task-workflow-governance.md).

## Key Decisions

- `grcl-platform` is the top-level architecture and integration repository.
- The existing `github.com/alibenD/grcl` repository remains an external C++/ROS2 prototype and a
  future `grcl-cpp` candidate.
- `grcl-c` is the planned core C contract boundary for runtime lifecycle, ownership, backend SPI,
  capability exchange, controlled storage, and cross-language consistency.
- Higher-level SDKs such as `grcl-cpp`, `grcl-py`, and future Rust bindings must wrap the core
  contract instead of defining independent runtime semantics.
- MCU/RTOS runtimes implement profile-based subsets rather than full GRCL by default.
- GRCL runtime participant graph and runtime capability exchange are first-class product
  infrastructure decisions.
- Runtime capability C ABI representation uses the recommended hybrid: fixed root structs for
  stable identity/protocol/profile/storage/lifecycle/result/graph summaries, and descriptors,
  iterators, or caller-provided output buffer APIs for variable transports, QoS, encodings,
  diagnostics, and security subsets.
- Opaque serialized blob support is deferred from G2 and remains a future protocol or
  management-plane extension point.
- Capability, availability, and health remain separate records and must not be collapsed into one
  public ABI object.
- ROS2 must not be described as exposing an rmw-level runtime middleware graph. ROS2 exposes a
  node graph to users while DDS/RTPS implementations provide lower-level participant and endpoint
  discovery.
- Workspace `artifacts/` is generated on demand and is not a pre-created or repository-owned
  directory.
- Implementation-stage work requires file-based task exchange plus an independent audit subagent
  gate before the main agent may proceed to the next task.

## Documentation Governance

- `docs/README.md` is the only global documentation navigation entrypoint.
- [System Overview](../architecture/system-overview.md) is the canonical design home for the whole
  middleware architecture.
- Each subsystem must have exactly one canonical design home. Supporting topic documents must be
  linked from that canonical home rather than becoming competing entrypoints.
- Before adding a design document, decide whether the content belongs in an existing canonical
  design home. New design documents must state their owning canonical home, why they cannot be
  merged into an existing document, and who is expected to maintain them.
- ADRs record decisions and consequences; they must not become duplicate API specifications.
- Plans record sequencing and validation; they must not become long-lived architecture narratives.
- Status documents record recovery state and gates; they must not become the primary explanation of
  middleware design.
- `.local/agentic-runs/` records execution evidence only and is not a human-facing documentation
  entrypoint.
- `docs/architecture/task-workflow-governance.md` is the repository source of truth for task
  classification, fast-path eligibility, TDD requirements, and `one task -> one commit`.

## Workspace Layout

Expected local layout:

```text
grcl-platform_ws/
  AGENTS.md
  src/
    grcl-platform/
  artifacts/        # generated on demand; may be absent or deleted
```

`artifacts/` is generated state. It is not a repository-owned directory and must be created on
demand by scripts before writing build, test, coverage, log, or generated files.

## Active Plans

- [Architecture Plan](../plans/2026-07-02-grcl-platform-architecture-plan.md)
- [Management Plane Architecture Plan](../plans/2026-07-03-management-plane-architecture-plan.md)
- [Module Skeleton Plan](../plans/2026-07-02-grcl-platform-module-skeleton-plan.md)
- [GRCL-C ABI Contract Plan](../plans/2026-07-03-grcl-c-abi-contract-plan.md)
- [Middleware Goal Roadmap](middleware-goal-roadmap.md)
- [M1 First Runnable GRCL-C Core Plan](../plans/2026-07-04-m1-first-runnable-core-plan.md)
- [G5 Conformance Harness Plan](../plans/2026-07-04-g5-conformance-harness-plan.md)
- [G6 Language SDK Wrapper Skeletons Plan](../plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md)
- [M3 Core Middleware Native Backend Plan](../plans/2026-07-04-m3-core-middleware-native-backend-plan.md)
- [M4 GRCL-C Core Contract Stabilization Plan](../plans/2026-07-05-m4-grcl-c-core-contract-stabilization-plan.md)
- [M5 Local Core Product And GRCL-CPP Completion Plan](../plans/2026-07-05-m5-local-core-product-and-grcl-cpp-completion-plan.md)

The architecture plan records the current system design baseline. The module skeleton plan is the
accepted G1 execution baseline for the current batch closeout. The middleware goal roadmap records
the dependency-ordered path from contract artifacts to release hardening, and future implementation
goals still require their own goal-specific plan and task briefs before execution.

## Important Architecture Entry Points

- [System Overview](../architecture/system-overview.md)
- [Repository Topology](../architecture/repository-topology.md)
- [Runtime Layering](../architecture/runtime-layering.md)
- [Runtime Capability Graph](../architecture/runtime-capability-graph.md)
- [Runtime Capability Schema](../architecture/runtime-capability-schema.md)
- [GRCL-C API Shape](../architecture/grcl-c-api-shape.md)
- [GRCL-C Capability ABI Representation](../architecture/grcl-c-capability-abi-representation.md)
- [MCU Runtime Profiles](../architecture/mcu-runtime-profiles.md)
- [Development Environment](../architecture/development-environment.md)
- [Container Workspace Strategy](../architecture/container-workspace-strategy.md)
- [Conformance And CI Strategy](../architecture/conformance-and-ci-strategy.md)
- [Management Plane Concept And Scope](../architecture/management-plane-concept-and-scope.md)
- [Management Plane Decision Scope](../architecture/management-plane-decision-scope.md)
- [Task Workflow Governance](../architecture/task-workflow-governance.md)
- [Agentic Delivery Governance](../architecture/agentic-delivery-governance.md)
- [Review And Verification Governance](../architecture/review-and-verification-governance.md)
- [Dependency Analysis](../architecture/dependency-analysis.md)
- [Agentic Delivery Templates](../templates/agentic-delivery/README.md)
- [Middleware Goal Roadmap](middleware-goal-roadmap.md)
- [Goal Execution Queue](goal-execution-queue.md)

## Open Questions

- Decide when and how to migrate, mirror, or leave external the current `grcl` C++/ROS2 prototype.

## Do Not Do Yet

- Do not implement runtime capability exchange.
- Do not treat G1 authorization as permission to start G2 without a G2-specific plan and task-brief sequence.
- Do not start any G2 batch without a narrowed task brief and an independently audited predecessor.
- Do not author a fake-resolved management-plane ADR before G10.
- Do not define authentication, remote management, event stream, or SDK permission-policy mechanisms
  as G1 requirements.
- Do not create runtime, SDK, backend, or build-system code without an accepted implementation plan.
- Do not execute implementation tasks without a file-based task brief and an independent audit
  subagent gate unless the user explicitly approves a bypass.
- Do not migrate `github.com/alibenD/grcl` into this repository without explicit confirmation.
- Do not assume macOS verifies ROS2 runtime behavior.
- Do not treat session history, subagent timeout, or unverified delegated review as completion
  evidence.
- Do not enter any post-M1 implementation milestone without a new approved implementation goal,
  task briefs, TDD plan, implementation subagent, and independent audit.
- Do not treat M1 completion as authorization to implement publish/subscribe, service/client,
  executor scheduling, transport, ROS2, MCU, SDK wrappers, management plane, auth, remote
  management, event streams, Docker, CI, repo-wide build-system policy, or external `grcl`
  migration.
- Do not treat G5 implementation approval as authorization for CI files, Dockerfiles, build-system
  files, SDK wrappers, pub/sub behavior, graph behavior, transport behavior, ROS2 behavior, MCU
  runtime behavior, simulator behavior, or management-plane behavior.

## Continuous Execution Contract

The user authorized goal-driven continuous execution for `G2-C` through `G2-E`, and that approved
window is now complete.

The main agent may not auto-advance beyond `G2-E` without a new user-approved goal window, even
though the completed window allowed per-batch advancement inside `G2-C` through `G2-E`.

The user later authorized the M1 Autonomous Goal Runner window, and that window is now complete. The
main agent may not auto-advance into any post-M1 implementation milestone without a new
user-approved goal window.

The user later authorized G5 Conformance Harness v0.1 planning, and then authorized G5
implementation under the goal-driven delivery model. That G5 implementation window is now complete
after G5-G independent audit acceptance. The main agent may not auto-advance into G6 or any other
post-G5 implementation milestone without a new user-approved goal window.

The user later authorized G6 Language SDK Wrapper Skeletons planning, selected Python binding
Option A, and then authorized goal-driven G6 implementation. That G6 implementation window is now
complete after G6-F closeout verification and independent audit acceptance. The main agent may not
auto-advance into G7 or any later milestone without a new user-approved goal window.

The user later authorized M3 Core Middleware Contract And Native In-Process Backend. That M3 window
is now complete after M3-J closeout audit acceptance and local conformance integration. M3 did not
authorize ROS2, DDS, sockets, shared memory, multi-process transport, simulator backend, MCU
runtime, gateway, management plane, auth, remote management, event streams, CI, Docker, package
manager, CMake, colcon, IDL/codegen, external `grcl` migration, or C++/Python example acceptance.

The user later authorized M4 GRCL-C Core Contract Stabilization And Conformance Expansion. M4 may
proceed through goal-specific planning, design review, TDD contract-hardening implementation
batches, local conformance expansion, and closeout inside the approved M4 scope. M4 does not
authorize ROS2, networking, simulator, MCU runtime, management plane, auth, remote management,
event streams, CI, Docker, package/build-system rollout, IDL/codegen, C++/Python example
acceptance, or external `grcl` migration.

The user later authorized M5 Local Core Product Surface And GRCL-CPP Completion. M5 may proceed
through goal-specific planning, design review, TDD implementation batches, local conformance
expansion, and closeout inside the approved local-core scope. M5 does not authorize ROS2, DDS,
networking, simulator, MCU runtime, gateway runtime, management plane, auth, remote management,
event streams, CI, Docker, package/build-system rollout, release packaging, IDL/codegen, external
`grcl` migration, or `grcl-py` expansion beyond the completed G6 boundary skeleton.

Stop conditions for this continuous window:

- independent audit returns `rejected`
- a new ABI, schema, or protocol decision is needed beyond the accepted G2 plan and ADR-0010
- a later batch would need to expand scope into ROS2, transport, threading, serialization,
  `grcl-py` expansion, management plane, build-system rollout, CI, or repo migration
- verification fails with no narrow corrective task available inside the current batch boundary
- the next batch needs a wider write set than its prepared brief and that widening changes product
  or architecture meaning

## New Session Bootstrap

From the workspace root:

```bash
pwd
git -C src/grcl-platform status --short --branch
git -C src/grcl-platform log -5 --oneline
sed -n '1,200p' AGENTS.md
sed -n '1,220p' src/grcl-platform/AGENTS.md
sed -n '1,220p' src/grcl-platform/README.md
sed -n '1,260p' src/grcl-platform/docs/status/current-context.md
```

After reading these files, inspect the active plan relevant to the user's request before editing.
