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
diagnostics core with a private null/native-test backend. No post-M1 implementation milestone is
authorized from the roadmap alone. G5 Conformance Harness v0.1 planning is complete and records a
local contract-drift harness scope; G5 implementation still requires a separate user-approved
implementation window.

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
- G5 implementation status: `pending_user_approval`
- G5 plan:
  [G5 Conformance Harness Plan](../plans/2026-07-04-g5-conformance-harness-plan.md)

## G2-G11 Refinement Rule

Before each later goal starts, the main agent must create or update a goal-specific plan that:

- derives requirements from artifacts produced by earlier goals.
- records allowed files and explicit non-goals.
- lists verification commands and acceptance criteria.
- chooses model and effort policy per task.
- defines the independent audit checklist.

Do not execute later goals from this roadmap alone. The roadmap establishes dependency order; each
goal still needs a task plan or task briefs before implementation. M1 completion does not authorize
G5 implementation, SDK, simulator, ROS2, MCU, gateway, management-plane, CI, Docker, or external
migration work. G5 planning completion does not authorize G5 implementation.

## Current Decision State

Management-plane concept work is no longer the blocking path for core middleware progress. It is
preserved as future input for G10. G1 is complete under the already authorized task-brief and
independent-audit workflow. G2-F closeout and G3 Backend SPI API Shape v0.1 design baseline are
complete. M1 First Runnable GRCL-C Core With Null/Native Backend is complete, locally verified, and
independently audited with notes. G5 Conformance Harness v0.1 planning is complete and should be
the next implementation milestone if the user approves implementation. No further auto-advance is
authorized from this roadmap state alone: any post-M1 implementation, including G5 implementation,
requires a new approved goal window with task briefs, TDD plan, implementation subagent, and
independent audit.
