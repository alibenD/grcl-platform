# GRCL Platform

GRCL Platform is the top-level architecture and integration repository for the GRCL middleware
ecosystem. It is the source of truth for system-level decisions, repository topology, runtime
layering, MCU/RTOS profiles, SDK governance, conformance planning, and development-environment
direction.

The existing `github.com/alibenD/grcl` repository remains the current C++/ROS2 package prototype
and is treated here as the future `grcl-cpp` module candidate, not as the whole middleware system.

## Current Status

- Architecture phase: `active`
- Code implementation phase: `paused`
- Primary current output: architecture documents and implementation planning
- Implementation requires user confirmation after reviewing the task plan

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
- [Workspace Bootstrap](docs/status/workspace-bootstrap.md)
- [Architecture Overview](docs/architecture/system-overview.md)
- [Repository Topology](docs/architecture/repository-topology.md)
- [Runtime Layering](docs/architecture/runtime-layering.md)
- [GRCL-C Contract Boundary](docs/architecture/grcl-c-contract-boundary.md)
- [GRCL-C API Shape](docs/architecture/grcl-c-api-shape.md)
- [Runtime Capability Graph](docs/architecture/runtime-capability-graph.md)
- [Runtime Capability Schema](docs/architecture/runtime-capability-schema.md)
- [MCU Runtime Profiles](docs/architecture/mcu-runtime-profiles.md)
- [MCU Profile Schema](docs/architecture/mcu-profile-schema.md)
- [Backend SPI Contract](docs/architecture/backend-spi-contract.md)
- [Language SDK Strategy](docs/architecture/language-sdk-strategy.md)
- [Development Environment](docs/architecture/development-environment.md)
- [Review And Verification Governance](docs/architecture/review-and-verification-governance.md)
- [SDK Platform Governance](docs/architecture/sdk-platform-governance.md)
- [Dependency Analysis](docs/architecture/dependency-analysis.md)
- [Task Plan](docs/plans/2026-07-02-grcl-platform-architecture-plan.md)
- [Module Skeleton Plan](docs/plans/2026-07-02-grcl-platform-module-skeleton-plan.md)

## Repository Policy

During architecture stabilization, this repository uses a modular monorepo strategy:

- Keep system-level ADRs and architecture docs in this repository.
- Keep modules separated by explicit public boundaries.
- Treat `grcl-c` as the core contract module.
- Treat `grcl-cpp`, `grcl-py`, and future language SDKs as wrappers over `grcl-c`.
- Keep runtime/backend capability rules visible before writing implementation code.
- Do not implement module code before the relevant architecture plan is reviewed and accepted.
