# ADR-0008 Backend SPI Contract

- Status: `draft`
- Date: `2026-07-02`

## Decision

Backend implementations must expose runtime, graph, capability, communication, scheduling, memory,
diagnostics, and management responsibilities through a `grcl-c`-owned SPI contract.

## Rationale

The platform needs backend replaceability without requiring identical internal scheduling, memory,
or transport implementations.

## Consequences

- ROS2, native, MCU, gateway, and simulator backends can differ internally.
- Public SDK behavior remains governed by `grcl-c`.
- Backend capability query is mandatory before exposing features to applications.

