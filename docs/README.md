# GRCL Platform Documentation

This directory is the documentation map for GRCL Platform. It is the only global documentation
navigation entrypoint. It should help readers find the right canonical design document without
creating parallel index or review-pack files.

## How To Read This Repository

| Reader goal | Start here | Then read |
|---|---|---|
| Understand the middleware architecture | [System Overview](architecture/system-overview.md) | [Runtime Layering](architecture/runtime-layering.md), [Repository Topology](architecture/repository-topology.md) |
| Review the `grcl-c` public contract | [GRCL-C API Shape](architecture/grcl-c-api-shape.md) | [GRCL-C Contract Boundary](architecture/grcl-c-contract-boundary.md), [GRCL-C Capability ABI Representation](architecture/grcl-c-capability-abi-representation.md) |
| Review runtime capability and graph semantics | [Runtime Capability Schema](architecture/runtime-capability-schema.md) | [Runtime Capability Graph](architecture/runtime-capability-graph.md), [Schema Artifact Index](../schemas/README.md) |
| Review backend and adapter direction | [Backend SPI Contract](architecture/backend-spi-contract.md) | [Runtime Layering](architecture/runtime-layering.md), [ADR-0008](adr/ADR-0008-backend-spi-contract.md) |
| Review SDK direction | [Language SDK Strategy](architecture/language-sdk-strategy.md) | [SDK Platform Governance](architecture/sdk-platform-governance.md), [ADR-0003](adr/ADR-0003-language-sdks-over-grcl-c.md) |
| Review M2/G6 SDK boundary baseline | [Language SDK Strategy](architecture/language-sdk-strategy.md) | [G6 Language SDK Wrapper Skeletons Plan](plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md), [SDK Platform Governance](architecture/sdk-platform-governance.md) |
| Review M3 core middleware design | [GRCL-C API Shape](architecture/grcl-c-api-shape.md) | [Backend SPI Contract](architecture/backend-spi-contract.md), [M3 Core Middleware Native Backend Plan](plans/2026-07-04-m3-core-middleware-native-backend-plan.md) |
| Run the local conformance harness | [Conformance And CI Strategy](architecture/conformance-and-ci-strategy.md) | `../scripts/run-conformance.sh`, [G5 Conformance Harness Plan](plans/2026-07-04-g5-conformance-harness-plan.md) |
| Recover current execution state | [Current Context](status/current-context.md) | [Goal Execution Queue](status/goal-execution-queue.md), [Middleware Goal Roadmap](status/middleware-goal-roadmap.md) |
| Audit a completed task | [Goal Execution Queue](status/goal-execution-queue.md) | `.local/agentic-runs/<plan-id>/<task-id>/ledger.md` and audit reports |

## Canonical Design Homes

Each subsystem has one long-lived design home. Add detail to the existing home before creating a
new architecture document. Supporting topic documents must be linked from their canonical home.

| Subsystem | Canonical design home | Supporting documents |
|---|---|---|
| Whole middleware architecture | [System Overview](architecture/system-overview.md) | [Runtime Layering](architecture/runtime-layering.md), [Repository Topology](architecture/repository-topology.md) |
| `grcl-c` core contract and API shape | [GRCL-C API Shape](architecture/grcl-c-api-shape.md) | [GRCL-C Contract Boundary](architecture/grcl-c-contract-boundary.md), [GRCL-C Capability ABI Representation](architecture/grcl-c-capability-abi-representation.md) |
| Runtime capability data contract | [Runtime Capability Schema](architecture/runtime-capability-schema.md) | [Runtime Capability Graph](architecture/runtime-capability-graph.md), [Schema Artifact Index](../schemas/README.md) |
| Backend SPI | [Backend SPI Contract](architecture/backend-spi-contract.md) | [ADR-0008](adr/ADR-0008-backend-spi-contract.md), future G3 plan |
| MCU and RTOS profiles | [MCU Runtime Profiles](architecture/mcu-runtime-profiles.md) | [MCU Profile Schema](architecture/mcu-profile-schema.md), [ADR-0007](adr/ADR-0007-mcu-runtime-profile-contract.md) |
| Language SDKs | [Language SDK Strategy](architecture/language-sdk-strategy.md) | [SDK Platform Governance](architecture/sdk-platform-governance.md), [ADR-0009](adr/ADR-0009-language-sdk-binding-contract.md) |
| Management plane | [Management Plane Concept And Scope](architecture/management-plane-concept-and-scope.md) | [Management Plane Decision Scope](architecture/management-plane-decision-scope.md), future G10 plan |
| Conformance and release gates | [Conformance And CI Strategy](architecture/conformance-and-ci-strategy.md) | [Review And Verification Governance](architecture/review-and-verification-governance.md) |
| Development workspace and artifacts | [Development Environment](architecture/development-environment.md) | [Container Workspace Strategy](architecture/container-workspace-strategy.md), [Workspace Bootstrap](status/workspace-bootstrap.md) |
| Agentic delivery process | [Agentic Delivery Governance](architecture/agentic-delivery-governance.md) | [Agentic Delivery Templates](templates/agentic-delivery/README.md) |

## Document Roles

| Document class | Owns | Must not own |
|---|---|---|
| `docs/architecture/` | Current design specifications and subsystem contracts | Step-by-step execution logs |
| `docs/adr/` | Decisions, rationale, consequences | Full API copies or evolving task status |
| `docs/plans/` | Sequencing, acceptance criteria, allowed files, validation | Long-lived design explanations that belong in architecture docs |
| `docs/status/` | Recovery state, goal queue, current gates | Complete architecture narratives |
| `schemas/` | Machine-readable or fixture-backed data contracts | Prose-only design rationale |
| `tests/conformance/` | Static conformance vectors and future validation inputs | Design entrypoint or review narrative |
| `.local/agentic-runs/` | Task briefs, implementation reports, audit reports, ledgers | Human-facing documentation navigation |

## Architecture Documents

Architecture documents are grouped by design area, not by creation order.

Core architecture:

- [System Overview](architecture/system-overview.md)
- [Runtime Layering](architecture/runtime-layering.md)
- [Repository Topology](architecture/repository-topology.md)
- [Dependency Analysis](architecture/dependency-analysis.md)

Core contract and capability:

- [GRCL-C Contract Boundary](architecture/grcl-c-contract-boundary.md)
- [GRCL-C API Shape](architecture/grcl-c-api-shape.md)
- [GRCL-C Capability ABI Representation](architecture/grcl-c-capability-abi-representation.md)
- [Runtime Capability Schema](architecture/runtime-capability-schema.md)
- [Runtime Capability Graph](architecture/runtime-capability-graph.md)

Runtime families, SDKs, and management:

- [Backend SPI Contract](architecture/backend-spi-contract.md)
- [MCU Runtime Profiles](architecture/mcu-runtime-profiles.md)
- [MCU Profile Schema](architecture/mcu-profile-schema.md)
- [Language SDK Strategy](architecture/language-sdk-strategy.md)
- [SDK Platform Governance](architecture/sdk-platform-governance.md)
- [Management Plane Concept And Scope](architecture/management-plane-concept-and-scope.md)
- [Management Plane Decision Scope](architecture/management-plane-decision-scope.md)

Environment, conformance, and delivery:

- [Development Environment](architecture/development-environment.md)
- [Container Workspace Strategy](architecture/container-workspace-strategy.md)
- [Conformance And CI Strategy](architecture/conformance-and-ci-strategy.md)
- Local runner: `../scripts/run-conformance.sh`
- SDK boundary drift checker: `../scripts/check-sdk-boundaries.py`
- [Agentic Delivery Governance](architecture/agentic-delivery-governance.md)
- [Review And Verification Governance](architecture/review-and-verification-governance.md)

## ADRs

- [ADR-0001 Modular Monorepo During Architecture Stabilization](adr/ADR-0001-modular-monorepo-during-architecture-stabilization.md)
- [ADR-0002 GRCL-C As Core Contract](adr/ADR-0002-grcl-c-as-core-contract.md)
- [ADR-0003 Language SDKs Over GRCL-C](adr/ADR-0003-language-sdks-over-grcl-c.md)
- [ADR-0004 Runtime Capability Exchange And Scoped Graph](adr/ADR-0004-runtime-capability-exchange-and-scoped-graph.md)
- [ADR-0005 GRCL-C Handle And Lifecycle Contract](adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md)
- [ADR-0006 Runtime Capability Schema](adr/ADR-0006-runtime-capability-schema.md)
- [ADR-0007 MCU Runtime Profile Contract](adr/ADR-0007-mcu-runtime-profile-contract.md)
- [ADR-0008 Backend SPI Contract](adr/ADR-0008-backend-spi-contract.md)
- [ADR-0009 Language SDK Binding Contract](adr/ADR-0009-language-sdk-binding-contract.md)
- [ADR-0010 GRCL-C Capability ABI Representation](adr/ADR-0010-grcl-c-capability-abi-representation.md)

## Plans And Status

Plans are execution artifacts. Read them after the relevant architecture documents.

- [GRCL Platform Architecture Plan](plans/2026-07-02-grcl-platform-architecture-plan.md)
- [GRCL Platform Module Skeleton Plan](plans/2026-07-02-grcl-platform-module-skeleton-plan.md)
- [GRCL Platform Management Plane Architecture Plan](plans/2026-07-03-management-plane-architecture-plan.md)
- [GRCL-C ABI Contract Plan](plans/2026-07-03-grcl-c-abi-contract-plan.md)
- [G5 Conformance Harness Plan](plans/2026-07-04-g5-conformance-harness-plan.md)
- [G6 Language SDK Wrapper Skeletons Plan](plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md)
- [M3 Core Middleware Native Backend Plan](plans/2026-07-04-m3-core-middleware-native-backend-plan.md)

Current recovery and queue state:

- [Current Context](status/current-context.md)
- [Goal Execution Queue](status/goal-execution-queue.md)
- [Middleware Goal Roadmap](status/middleware-goal-roadmap.md)
- [Workspace Bootstrap](status/workspace-bootstrap.md)

## Current Artifacts

- `grcl-c` public headers: `../src/grcl-c/include/grcl/c/`
- `grcl-c` compile-only smoke tests: `../src/grcl-c/tests/`
- Schema artifacts: [Schema Artifact Index](../schemas/README.md)
- Runtime capability fixtures: `../tests/conformance/runtime-capability/`
- MCU profile fixtures: `../tests/conformance/mcu-profiles/`

## Diagrams

- [System Overview Mermaid](assets/system-overview.mmd)
- [Repository Topology Mermaid](assets/repository-topology.mmd)
- [Runtime Graph Mermaid](assets/runtime-graph.mmd)
