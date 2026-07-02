# Dependency Analysis

## Purpose

This document captures the dependency order required before implementation starts. It prevents
writing module code before the upstream contracts are stable enough to support it.

## Architecture Dependency Order

```text
Repository topology
  -> system architecture diagrams
    -> ADR baseline
      -> grcl-c contract boundary
        -> runtime graph and capability schema
          -> MCU runtime profiles
            -> backend SPI
              -> language SDK strategy
                -> conformance and CI strategy
                  -> container workspace strategy
                    -> implementation tasks
```

## Critical Dependencies

| Downstream Work | Required Upstream Decision |
|---|---|
| `grcl-c` API implementation | handle set, lifecycle, error model, storage model |
| `grcl-cpp` migration | `grcl-c` wrapper policy and module boundary |
| `grcl-py` design | binding policy and C ABI shape |
| MCU runtime | MCU profiles, static storage, transport adapter, graph depth |
| runtime graph | capability schema and projection modes |
| backend SPI | runtime host contract and capability query shape |
| wire protocol | runtime hello, capability exchange, channel negotiation result |
| SDK governance | manifest, permission, resource, audit, conformance policy |
| Docker/CI | module topology, verification matrix, artifact root policy |
| module skeleton execution | conformance and CI strategy, container workspace strategy, agentic delivery governance |

## Current Blockers

- No `grcl-c` module exists yet.
- Detailed `grcl-c` handle/API schema exists as architecture documentation, but no header or
  module implementation exists yet.
- MCU/RTOS profile schema exists as architecture documentation, but no conformance tests or module
  implementation exist yet.
- Runtime capability schema exists as architecture documentation, but no C structs, wire frames,
  or management API output exist yet.
- Docker/Ubuntu verification environment is not initialized in this repository.
- Build artifact path policy exists as architecture documentation, but scripts enforcing
  `GRCL_PLATFORM_ARTIFACT_ROOT` do not exist yet.
- Conformance and CI strategy exists as architecture documentation, but no schemas, fixtures,
  scripts, or CI jobs exist yet.
- Container workspace strategy exists as architecture documentation, but no Dockerfiles or
  dev-shell scripts exist yet.
- Current `github.com/alibenD/grcl` is still an external implementation repository, not migrated.

## Implementation Dependency Graph

| Phase | Must Exist Before Phase Starts | Produces |
|---|---|---|
| module skeletons | repository topology accepted | directories, module READMEs, package metadata |
| `grcl-c` headers | API shape accepted | public C headers, ABI version constants |
| `grcl-c` lifecycle tests | headers and result model | lifecycle conformance tests |
| runtime capability structs | capability schema accepted | C structs and serialization test vectors |
| MCU profile enforcement | profile schema and controlled storage | profile validators and static limits |
| backend SPI | lifecycle and capability query | backend registration and dispatch tables |
| language SDK wrappers | `grcl-c` lifecycle and node handles | C++/Python wrapper skeletons |
| dev containers | module layout, verification matrix, artifact root policy, container workspace strategy | Dockerfiles and scripts |
| conformance suite | core contract and schemas | cross-module validation |

## Recommended Next Planning Sequence

1. Finalize repository topology and module boundaries.
2. Finalize system diagrams.
3. Define `grcl-c` contract schema.
4. Define runtime capability record and graph projection schemas.
5. Define MCU runtime profile schema.
6. Define backend SPI boundary.
7. Define language SDK binding policies at file/API level.
8. Define conformance and CI matrix. Completed as
   [Conformance And CI Strategy](conformance-and-ci-strategy.md).
9. Define container workspace and Docker image boundaries. Completed as
   [Container Workspace Strategy](container-workspace-strategy.md).
10. Only then implement module skeletons.

## Implementation Plan Output

The first implementation plan derived from this dependency analysis is:

- [GRCL Platform Module Skeleton Plan](../plans/2026-07-02-grcl-platform-module-skeleton-plan.md)

It is blocked until user confirmation.
