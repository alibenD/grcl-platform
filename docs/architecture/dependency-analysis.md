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
                -> conformance plan
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
| Docker/CI | module topology and verification matrix |

## Current Blockers

- No `grcl-c` module exists yet.
- No detailed `grcl-c` handle/API schema exists yet.
- No profile schema exists for MCU/RTOS targets.
- No capability record schema exists.
- Docker/Ubuntu verification environment is not initialized in this repository.
- Current `github.com/alibenD/grcl` is still an external implementation repository, not migrated.

## Recommended Next Planning Sequence

1. Finalize repository topology and module boundaries.
2. Finalize system diagrams.
3. Define `grcl-c` contract schema.
4. Define runtime capability record and graph projection schemas.
5. Define MCU runtime profile schema.
6. Define backend SPI boundary.
7. Define language SDK binding policies at file/API level.
8. Define conformance and CI matrix.
9. Only then implement module skeletons.

