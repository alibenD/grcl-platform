# ADR-0001 Modular Monorepo During Architecture Stabilization

- Status: `accepted`
- Date: `2026-07-02`

## Decision

GRCL Platform uses a modular monorepo strategy during architecture stabilization.

The top-level `grcl-platform` repository is the source of truth for:

- system architecture and ADRs
- module topology
- cross-module contracts
- development environment direction
- integration and conformance planning
- task plans for staged implementation

Individual modules must be designed with clear public boundaries so they can be split into
separate release repositories or public mirrors later if needed.

## Rationale

GRCL is still defining core contracts across `grcl-c`, language SDKs, backend SPI, MCU/RTOS
profiles, IDL/codegen, runtime graph, gateway behavior, and SDK governance. These areas are highly
coupled in the architecture phase. Splitting them into independent repositories before contracts
stabilize would increase semantic drift and make cross-module changes expensive.

The current `github.com/alibenD/grcl` repository remains the current C++/ROS2 package prototype and
is treated as a future `grcl-cpp` module candidate. It should not be treated as the full GRCL
middleware platform.

## Consequences

- Architecture changes can update multiple module contracts atomically.
- Integration and conformance planning can be centralized.
- Future repo split remains possible because modules must keep explicit boundaries.
- Release independence is deferred until API/ABI boundaries become stable.

