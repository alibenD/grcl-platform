# Middleware Goal Roadmap

## Purpose

This document is the durable goal roadmap for turning GRCL Platform from architecture documents
into an implementation-ready middleware and then into a runnable middleware stack. It exists to
prevent open-ended design loops: every goal must produce concrete artifacts, have explicit exit
criteria, and hand off to the next goal only after verification.

This roadmap is ordered by dependency. Later goals may be refined as earlier goals produce real
contract artifacts and implementation feedback.

## Current State

Completed implementation milestone:

```text
M1: First Runnable GRCL-C Core With Null/Native Backend
```

G1 turned accepted architecture into repository artifacts: module skeletons, `grcl-c` public header
skeletons, schema files, and conformance fixtures. It does not implement runtime behavior. G1-A
through G1-E are produced and independently audited. G2 planning is filed, and G2-A resolved the
runtime capability C ABI representation gate as the recommended hybrid and passed independent audit.
G2-B header hardening, G2-C runtime declaration hardening, and G2-D capability query and
negotiation ABI shaping also passed independent audit. G2-E compile-only smoke artifacts also
passed independent audit. The approved goal-driven execution window stopped at G2-E and is now
complete. Middleware documentation information architecture cleanup is also complete. The next
design-only goal closed G2-F and produced the G3 Backend SPI API Shape v0.1 baseline. M1 has now
completed the first runnable `grcl-c` lifecycle, bounded-storage, capability negotiation, and
diagnostics core with a private null/native-test backend. G5 Conformance Harness v0.1 is complete
and provides the local contract-drift harness. G6 Language SDK Wrapper Skeletons is complete as M2
Cross-Language SDK Boundary Baseline: `grcl-cpp` and `grcl-py` wrapper skeletons exist, Python
binding Option A is recorded, and SDK boundary drift checks are integrated into the local
conformance runner. M3 Core Middleware Contract And Native In-Process Backend is complete under a
goal-specific plan and is limited to local in-process native backend behavior for C examples. M4
GRCL-C Core Contract Stabilization And Conformance Expansion is complete after closeout audit
acceptance. M5 Local Core Product Surface And GRCL-CPP Completion is now the active milestone: it
targets a feature-complete native-backend local-core product surface in `grcl-c`, full `grcl-cpp`
wrapping of that surface, and passing C/C++ tests plus runnable C/C++ examples.

## Model And Effort Policy

| Work Type | Default Policy |
|---|---|
| Architecture design, ADRs, cross-goal planning | GPT-5.5, high effort |
| Implementation planning and task-brief writing | GPT-5.5, high effort |
| Narrow implementation subagent work | GPT-5.4, medium effort by default |
| ABI, schema, protocol, backend SPI, concurrency, safety, or cross-module implementation | Escalate to GPT-5.5 or high effort |
| Independent audit subagent | GPT-5.5, high effort by default |

Implementation models do not need to be maximized for every task. The main agent must choose the
lowest model and effort level that can still meet the architecture, ABI, conformance, and quality
bar for the task, and must record the chosen policy in each task brief.

## Execution Governance

- Design and ADR work may be done in the main session when it is documentation-only.
- Implementation work must follow [Agentic Delivery Governance](../architecture/agentic-delivery-governance.md).
- The main agent owns queue control, task-brief creation, subagent dispatch, independent audit, and
  durable status updates.
- Implementation subagents receive work through
  `.local/agentic-runs/<plan-id>/<task-id>/task-brief.md`.
- Every implementation result must be followed by an independent audit subagent before the next
  implementation task starts.
- A failed audit creates a narrow fix task and a new audit loop.

## Documentation Information Architecture

- [Documentation README](../README.md) is the only global documentation navigation entrypoint.
- [System Overview](../architecture/system-overview.md) is the canonical design home for the whole
  middleware architecture.
- Every subsystem goal must maintain or update its existing canonical design home. A goal must not
  leave its durable design only in a plan, status file, task brief, implementation report, or audit
  report.
- New architecture documents are allowed only when the content cannot be cleanly owned by an
  existing canonical design home. The new document must be reachable from the canonical home.
- ADRs, plans, status files, schemas, tests, and `.local/agentic-runs` evidence must preserve their
  narrow roles and must not become competing design entrypoints.

## Goal Sequence

| Goal | Name | Produces | Exit Criteria |
|---|---|---|---|
| G1 | v0.1 Contract Artifacts | module skeletons, `grcl-c` header skeletons, schema artifacts, conformance fixtures | Contract files exist; header hygiene passes; schema and fixture inventory exists; no runtime behavior implemented |
| G2 | `grcl-c` ABI Contract v0.1 | compile-oriented C ABI headers, result model, opaque handles, lifecycle, storage, capability query | C headers compile in a minimal test; no C++/ROS2 public type leakage; ABI rules are explicit |
| G3 | Backend SPI API Shape v0.1 | backend registration design, C function-table shape, capability query hooks, graph/diagnostics hooks | Backend boundary is precise enough to brief G4 implementation without deciding ROS2 internals |
| G4 | Null/Native Test Backend v0.1 | superseded at the minimum runnable-core level by M1; any expanded backend work needs a new plan | M1 closeout verified create/start/stop/destroy, capability query, negotiation, diagnostics, and bounded-storage negative paths against a null/native-test backend |
| G5 | Conformance Harness v0.1 | local docs, schema, C ABI, capability fixture, MCU profile fixture, and M1 harness checks | Local checks can fail on contract drift before SDK, simulator, ROS2, MCU runtime, management-plane, CI, or release work expands |
| G6 | Language SDK Wrapper Skeletons | `grcl-cpp` and `grcl-py` wrappers over `grcl-c` handles | SDK skeletons wrap core handles without independent lifecycle semantics |
| M3 | Core Middleware Contract And Native In-Process Backend | sub/pub, service/client, executor pull, local runtime params, C examples, native in-process backend | C examples run through local backend and conformance without transport or ROS2 scope |
| M4 | GRCL-C Core Contract Stabilization And Conformance Expansion | negative-path C contract tests, boundary hardening, dedicated M4 runner, local conformance stage | M3 `grcl-c` local core behavior is backed by contract tests and conformance without adding new feature areas |
| M5 | Local Core Product Surface And GRCL-CPP Completion | completed native-backend `grcl-c` local-core surface, full `grcl-cpp` wrapper, C/C++ tests, C/C++ examples, local C++ conformance stage | native-backend local core is usable from both C and C++ without independent SDK semantics or out-of-scope platform expansion |
| G7 | Simulator Runtime v0.1 | deterministic simulator backend for graph, capability, diagnostics, and snapshot validation | Middleware semantics can be tested without ROS2 or hardware |
| G8 | ROS2 Adapter Design And Skeleton | ROS2 backend skeleton, graph projection rules, `rcl`/`rmw`/`rclcpp` containment | ROS2 types remain backend-private; GRCL runtime participant mapping is explicit |
| G9 | MCU And Gateway Profile Runtime | profile-limited MCU adapters and gateway representation path | MCU profiles avoid full-graph assumptions; gateway representation is testable |
| G10 | Management Plane Snapshot Contract | snapshot schema, CLI/debug output shape, health/capability/graph/diagnostics view | Management plane becomes a verifiable snapshot contract, not remote control or auth |
| G11 | Integration And Release Hardening | release conformance matrix, versioning policy, migration plan for external `grcl` | v0.1 platform baseline can be named with evidence and known limits |

## G1 Scope

G1 includes:

- module skeleton directories and README files.
- `grcl-c` public header skeletons for version, result, types, runtime, capability, and storage.
- runtime capability, MCU profile, and capability negotiation schema artifacts.
- conformance fixture skeletons for accepted, degraded, rejected, and MCU profile examples.
- documentation index updates and validation evidence.

G1 excludes:

- runtime behavior.
- publish, subscribe, service, executor, or transport implementation.
- backend SPI function-table implementation.
- native, null, simulator, ROS2, MCU, or gateway backend implementation.
- C++ or Python SDK implementation.
- management plane implementation.
- authentication, remote management, event streams, or SDK permission policy.
- migration of `github.com/alibenD/grcl`.

## G1 Batch Breakdown

| Batch | Work | Requires Audit | Notes |
|---|---|---|---|
| G1-A | module skeleton directories and README files | yes | Narrow mechanical task; GPT-5.4 medium is sufficient unless architecture drift is found |
| G1-B | `grcl-c` public header skeletons | yes | ABI-sensitive; use stronger model/effort or escalate during audit |
| G1-C | schema artifacts | yes | Schema/protocol-sensitive; use stronger model/effort if field semantics are ambiguous |
| G1-D | conformance fixtures | yes | Must map back to schema and architecture docs |
| G1-E | documentation and validation closeout | yes | Must prove implementation pause was respected and no runtime behavior was added |

Current G1 status:

- G1-A audit status: `accepted_with_notes`
- G1-B audit status: `accepted_with_notes`
- G1-C audit status: `accepted_with_notes`
- G1-D audit status: `accepted`
- G1-E audit status: `accepted`

Deferred from the original module skeleton plan:

- Original Task 5 development-environment scaffolding remains deferred.
- `scripts/test-docs.sh`, `scripts/check-links.py`, `scripts/env.sh`, `docker/ubuntu-ros-humble/`, and `docker/mcu-cross/` are outside the approved G1 scope because G1 currently forbids build scripts and Dockerfiles.

## G5 Scope

G5 Conformance Harness v0.1 is planned as a local contract-drift harness. It includes:

- documentation consistency checks.
- schema fixture validation for runtime capability and capability negotiation.
- MCU profile fixture validation.
- C ABI/header conformance checks.
- integration of the existing M1 runnable harness.
- artifact-root enforcement.

G5 excludes:

- CI workflows, Dockerfiles, containers, package-manager policy, and repo-wide build-system policy.
- SDK wrapper implementation.
- publish/subscribe, service/client, graph behavior, graph deltas, transport, executor scheduling,
  ROS2 behavior, MCU runtime behavior, simulator behavior, and management-plane behavior.
- authentication, remote management, event streams, release signing, registry work, and external
  `github.com/alibenD/grcl` migration.

Current G5 status:

- G5 planning status: `complete`
- G5 implementation status: `complete`
- G5 plan:
  [G5 Conformance Harness Plan](../plans/2026-07-04-g5-conformance-harness-plan.md)

## G6 Scope

G6 Language SDK Wrapper Skeletons is planned as M2 Cross-Language SDK Boundary Baseline. It
includes:

- `grcl-cpp` RAII wrapper skeleton over `grcl-c` handles.
- `grcl-py` ownership wrapper skeleton over a private native boundary shaped like `grcl-c`.
- SDK boundary drift checks.
- local verification that composes with the G5 conformance runner.

G6 excludes:

- full SDK usability claims.
- pub/sub, service/client, executor scheduling, graph behavior, transport, ROS2 adapter behavior,
  MCU runtime behavior, simulator behavior, management-plane behavior, auth, remote management, and
  event streams.
- package distribution, CI, Docker, repo-wide build-system rollout, external `grcl` migration, and
  native extension framework selection unless separately approved.

Current G6 status:

- G6 planning status: `complete`
- G6 implementation status: `complete`
- G6 Python binding substrate: Option A, `ctypes` or private dynamic-library shim boundary with
  package distribution deferred
- G6 implementation evidence: G6-C audit `accepted_with_notes`, G6-D audit `accepted`, and G6-E
  audit `accepted` under `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/`
- G6 plan:
  [G6 Language SDK Wrapper Skeletons Plan](../plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md)

## G2-G11 Refinement Rule

Before each later goal starts, the main agent must create or update a goal-specific plan that:

- derives requirements from artifacts produced by earlier goals.
- records allowed files and explicit non-goals.
- lists verification commands and acceptance criteria.
- chooses model and effort policy per task.
- defines the independent audit checklist.

Do not execute later goals from this roadmap alone. The roadmap establishes dependency order; each
goal still needs a task plan or task briefs before implementation. G6 completion does not authorize
simulator, ROS2, MCU, gateway, management-plane, CI, Docker, package/build policy, external
migration, or expanded SDK behavior beyond the completed wrapper skeleton baseline.

## M3 Scope

M3 Core Middleware Contract And Native In-Process Backend includes:

- `grcl-c` node, publisher, subscription, service, client, executor, type-support, and local
  runtime parameter contract.
- backend SPI v0.2 append-only operation table design.
- in-process native backend routing for local pub/sub and service/client examples.
- runtime-local parameter table.
- C examples and local conformance integration.

M3 excludes ROS2, DDS, sockets, shared memory, multi-process transport, simulator backend, MCU
runtime, gateway, management plane, auth, remote management, event streams, CI, Docker, package
manager, CMake, colcon, IDL/codegen, external `grcl` migration, and C++/Python example acceptance.

Current M3 status:

- M3 planning status: `complete`
- M3 design review status: `complete`
- M3 implementation status: `complete after M3-J conformance integration and closeout audit`
- M3 plan:
  [M3 Core Middleware Native Backend Plan](../plans/2026-07-04-m3-core-middleware-native-backend-plan.md)

## Current Decision State

Management-plane concept work is no longer the blocking path for core middleware progress. It is
preserved as future input for G10. G1 is complete under the already authorized task-brief and
independent-audit workflow. G2-F closeout and G3 Backend SPI API Shape v0.1 design baseline are
complete. M1 First Runnable GRCL-C Core With Null/Native Backend is complete, locally verified, and
independently audited with notes. G5 Conformance Harness v0.1 is complete after independent audit
acceptance. G6 Language SDK Wrapper Skeletons is complete after task briefs, TDD-style verification,
implementation subagents, and independent audit. M3 Core Middleware Contract And Native In-Process
Backend is complete under a goal-specific plan. M4 GRCL-C Core Contract Stabilization And
Conformance Expansion is complete after closeout audit acceptance. M5 Local Core Product Surface
And GRCL-CPP Completion is the current goal and requires its own design review, task briefs,
implementation subagents, and independent audits.

## M4 Scope

M4 GRCL-C Core Contract Stabilization And Conformance Expansion includes:

- negative-path and boundary tests for the completed M3 `grcl-c` local core surface.
- lifecycle, ownership, destroyed-handle, cross-runtime, and deterministic cleanup contract checks.
- messaging, executor, params, and capability contract checks.
- a dedicated M4 local contract runner and top-level conformance integration.
- narrow production fixes only when a failing contract test proves drift from the documented M3
  contract.

M4 excludes ROS2, DDS, sockets, shared memory, networking, multi-process transport, simulator
backend, MCU runtime, gateway, management plane, auth, remote management, event streams, CI,
Docker, package manager, CMake, colcon, IDL/codegen, external `grcl` migration, C++/Python example
acceptance, new public feature areas, and release-stability claims.

Current M4 status:

- M4 planning status: `complete`
- M4 design review status: `complete after API re-review accepted; backend and test-plan reviews accepted with notes`
- M4 implementation status: `complete after M4-G closeout audit accepted with notes`
- M4 plan:
  [M4 GRCL-C Core Contract Stabilization Plan](../plans/2026-07-05-m4-grcl-c-core-contract-stabilization-plan.md)

## M5 Scope

M5 Local Core Product Surface And GRCL-CPP Completion includes:

- completion of the native-backend local-core `grcl-c` surface as the semantic source of truth:
  runtime, node, publisher, subscription, service, client, executor, type support,
  runtime-local params, capability, and diagnostics.
- full `grcl-cpp` wrapping of that approved local-core surface.
- local C and C++ tests plus runnable C and C++ examples.
- top-level local conformance expansion for the approved C++ parity surface.

M5 excludes ROS2, DDS, sockets, shared memory, networking, multi-process transport, simulator
backend, MCU runtime, gateway runtime, management plane, auth, remote management, event streams,
CI, Docker, package manager, CMake, colcon, release packaging, external `grcl` migration,
IDL/codegen, and `grcl-py` expansion beyond the completed G6 boundary skeleton.

Current M5 status:

- M5 authorization status: `complete`
- M5 planning status: `M5-A and M5-B complete`
- M5 implementation status: `M5-C complete; M5-D grcl-cpp runtime/node/executor batch is the next gate`
- M5 plan:
  [M5 Local Core Product And GRCL-CPP Completion Plan](../plans/2026-07-05-m5-local-core-product-and-grcl-cpp-completion-plan.md)
