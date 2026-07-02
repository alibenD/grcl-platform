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

- Architecture design is active.
- Code implementation is paused.
- Module skeleton implementation requires explicit user confirmation.
- Runtime capability exchange is a design decision, not an implemented runtime feature.
- Architecture design, ADR writing, and task planning should use GPT-5.5 high effort when
  available.
- Future implementation tasks should use implementation subagents plus independent audit subagents
  according to the agentic delivery governance document.

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
- ROS2 must not be described as exposing an rmw-level runtime middleware graph. ROS2 exposes a
  node graph to users while DDS/RTPS implementations provide lower-level participant and endpoint
  discovery.
- Workspace `artifacts/` is generated on demand and is not a pre-created or repository-owned
  directory.
- Implementation-stage work requires file-based task exchange plus an independent audit subagent
  gate before the main agent may proceed to the next task.

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
- [Module Skeleton Plan](../plans/2026-07-02-grcl-platform-module-skeleton-plan.md)

The architecture plan records the current system design baseline. The module skeleton plan is a
candidate execution plan and must not be implemented until explicitly accepted.

## Important Architecture Entry Points

- [System Overview](../architecture/system-overview.md)
- [Repository Topology](../architecture/repository-topology.md)
- [Runtime Layering](../architecture/runtime-layering.md)
- [Runtime Capability Graph](../architecture/runtime-capability-graph.md)
- [Runtime Capability Schema](../architecture/runtime-capability-schema.md)
- [MCU Runtime Profiles](../architecture/mcu-runtime-profiles.md)
- [Development Environment](../architecture/development-environment.md)
- [Agentic Delivery Governance](../architecture/agentic-delivery-governance.md)
- [Review And Verification Governance](../architecture/review-and-verification-governance.md)
- [Dependency Analysis](../architecture/dependency-analysis.md)
- [Agentic Delivery Templates](../templates/agentic-delivery/README.md)

## Open Questions

- Decide when to accept or revise the module skeleton plan.
- Decide when to create the first `grcl-c` module skeleton.
- Decide when and how to migrate, mirror, or leave external the current `grcl` C++/ROS2 prototype.
- Decide the first Docker image boundary for Ubuntu native, ROS2, and future MCU cross-build
  validation.

## Do Not Do Yet

- Do not implement runtime capability exchange.
- Do not create runtime, SDK, backend, or build-system code without an accepted implementation plan.
- Do not execute implementation tasks without a file-based task brief and an independent audit
  subagent gate unless the user explicitly approves a bypass.
- Do not migrate `github.com/alibenD/grcl` into this repository without explicit confirmation.
- Do not assume macOS verifies ROS2 runtime behavior.
- Do not treat session history, subagent timeout, or unverified delegated review as completion
  evidence.

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
