# GRCL Platform

GRCL Platform is the top-level architecture and integration repository for the GRCL middleware
ecosystem. It is the source of truth for system-level decisions, repository topology, runtime
layering, MCU/RTOS profiles, SDK governance, conformance planning, and development-environment
direction.

The existing `github.com/alibenD/grcl` repository remains the current C++/ROS2 package prototype
and is treated here as the future `grcl-cpp` module candidate, not as the whole middleware system.

## Current Status

- Architecture phase: `active`
- Runtime/product code implementation phase: `paused`
- Bounded G1 contract-artifact creation: `complete under task briefs and independent audit`
- G2 `grcl-c` ABI Contract v0.1: `complete`
- G3 Backend SPI API Shape v0.1 design baseline: `complete`
- M1 First Runnable GRCL-C Core With Null/Native Backend: `complete under task briefs, TDD-style verification, and independent audit`
- Primary current output: M1 runnable-core closeout; next milestone requires explicit user approval
- Autonomous Goal Runner mode completed for M1 and must not auto-advance into post-M1 implementation
- G1-A through G1-E passed independent audit
- M1-B through M1-G passed independent audit, with M1-G accepted with non-blocking repository
  hygiene notes

## Recommended Local Layout

Use an explicit workspace layout for local development and long-running Codex sessions:

```text
grcl-platform_ws/
  AGENTS.md
  artifacts/        # generated on demand; may be absent or deleted
  src/
    grcl-platform/
```

Prefer opening Codex at `grcl-platform_ws` so Docker mounts, generated artifacts, and future
multi-repository orchestration share one workspace boundary. The Git repository remains
`grcl-platform_ws/src/grcl-platform`; repository commands from the workspace root should use
`git -C src/grcl-platform ...`.

Build and test scripts must write generated files under `grcl-platform_ws/artifacts` by default,
unless `GRCL_PLATFORM_ARTIFACT_ROOT` is set explicitly. `artifacts/` is generated state and scripts
must create it on demand before writing outputs.

## Document Entry Points

- [Current Context](docs/status/current-context.md)
- [Goal Execution Queue](docs/status/goal-execution-queue.md)
- [Middleware Goal Roadmap](docs/status/middleware-goal-roadmap.md)
- [Workspace Bootstrap](docs/status/workspace-bootstrap.md)
- [Schema Artifact Index](schemas/README.md)
- `grcl-c` contract headers and M1 tests: `src/grcl-c/`
- Conformance fixture directories: `tests/conformance/runtime-capability/`, `tests/conformance/mcu-profiles/`
- [Architecture Overview](docs/architecture/system-overview.md)
- [Repository Topology](docs/architecture/repository-topology.md)
- [Runtime Layering](docs/architecture/runtime-layering.md)
- [GRCL-C Contract Boundary](docs/architecture/grcl-c-contract-boundary.md)
- [GRCL-C API Shape](docs/architecture/grcl-c-api-shape.md)
- [GRCL-C Capability ABI Representation](docs/architecture/grcl-c-capability-abi-representation.md)
- [Runtime Capability Graph](docs/architecture/runtime-capability-graph.md)
- [Runtime Capability Schema](docs/architecture/runtime-capability-schema.md)
- [MCU Runtime Profiles](docs/architecture/mcu-runtime-profiles.md)
- [MCU Profile Schema](docs/architecture/mcu-profile-schema.md)
- [Backend SPI Contract](docs/architecture/backend-spi-contract.md)
- [Language SDK Strategy](docs/architecture/language-sdk-strategy.md)
- [Development Environment](docs/architecture/development-environment.md)
- [Container Workspace Strategy](docs/architecture/container-workspace-strategy.md)
- [Conformance And CI Strategy](docs/architecture/conformance-and-ci-strategy.md)
- [Management Plane Concept And Scope](docs/architecture/management-plane-concept-and-scope.md)
- [Management Plane Decision Scope](docs/architecture/management-plane-decision-scope.md)
- [Agentic Delivery Governance](docs/architecture/agentic-delivery-governance.md)
- [Review And Verification Governance](docs/architecture/review-and-verification-governance.md)
- [SDK Platform Governance](docs/architecture/sdk-platform-governance.md)
- [Dependency Analysis](docs/architecture/dependency-analysis.md)
- [Agentic Delivery Templates](docs/templates/agentic-delivery/README.md)
- [Task Plan](docs/plans/2026-07-02-grcl-platform-architecture-plan.md)
- [Management Plane Plan](docs/plans/2026-07-03-management-plane-architecture-plan.md)
- [Module Skeleton Plan](docs/plans/2026-07-02-grcl-platform-module-skeleton-plan.md)
- [GRCL-C ABI Contract Plan](docs/plans/2026-07-03-grcl-c-abi-contract-plan.md)
- [M1 First Runnable GRCL-C Core Plan](docs/plans/2026-07-04-m1-first-runnable-core-plan.md)
- [ADR-0010 GRCL-C Capability ABI Representation](docs/adr/ADR-0010-grcl-c-capability-abi-representation.md)

## Repository Policy

During architecture stabilization, this repository uses a modular monorepo strategy:

- Keep system-level ADRs and architecture docs in this repository.
- Keep modules separated by explicit public boundaries.
- Treat `grcl-c` as the core contract module.
- Treat `grcl-cpp`, `grcl-py`, and future language SDKs as wrappers over `grcl-c`.
- Keep runtime/backend capability rules visible before writing implementation code.
- Do not implement module code before the relevant architecture plan is reviewed and accepted.
