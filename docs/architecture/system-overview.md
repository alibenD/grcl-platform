# GRCL Platform System Overview

## Purpose

GRCL is a heterogeneous robot middleware platform. It must support Linux hosts, Jetson-class
edge compute, ROS2 interop, native runtimes, gateway nodes, MCU/RTOS runtimes, simulators, tools,
and future third-party SDKs while preserving one coherent programming and runtime model.

This document is the canonical design home for the whole middleware architecture. It explains how
the major subsystems relate to one another and links to the narrower design homes that own each
subsystem's detailed contract.

## Top-Level Architecture

```text
Applications / Robot Features / Third-Party SDKs
  -> Language SDKs: grcl-cpp, grcl-py, grcl-rs, grcl-c
    -> GRCL-C Core Contract
      -> Backend SPI and Runtime Host Contracts
        -> Native Runtime
        -> ROS2 Adapter Runtime
        -> MCU/RTOS Runtime Profiles
        -> Gateway Runtime
        -> Simulator Runtime
```

The architecture has six design layers:

| Layer | Responsibility | Canonical design home |
|---|---|---|
| SDK layer | User-facing language APIs that wrap the core contract | [Language SDK Strategy](language-sdk-strategy.md) |
| `grcl-c` core contract | Stable C ABI for lifecycle, ownership, results, storage, and capability exchange | [GRCL-C API Shape](grcl-c-api-shape.md) |
| Runtime host layer | Runtime participant, node ownership, endpoint ownership, and executor integration concepts | [Runtime Layering](runtime-layering.md) |
| Backend SPI layer | Adapter boundary for native, ROS2, MCU/RTOS, gateway, and simulator runtimes | [Backend SPI Contract](backend-spi-contract.md) |
| Capability and graph layer | Runtime capability, availability, health, graph projection, and negotiation contracts | [Runtime Capability Schema](runtime-capability-schema.md) |
| Conformance and management layer | Validation, observability, future snapshots, and review gates | [Conformance And CI Strategy](conformance-and-ci-strategy.md) |

## System Goals

- Keep core runtime semantics consistent across MCU, Linux, ROS2, native, gateway, and simulator
  deployments.
- Allow MCU/RTOS runtimes to implement profile-based subsets rather than full desktop runtime
  behavior.
- Make runtime capability exchange explicit before cross-runtime channels are opened.
- Keep language SDKs convenient without allowing them to define conflicting core semantics.
- Support future SDK governance, permissions, conformance tests, observability, and registry
  workflows.

## Non-Goals For The Current Phase

- Do not implement runtime code in this repository yet.
- Do not migrate the current `github.com/alibenD/grcl` C++ package until repository topology is
  reviewed.
- Do not create a public SDK marketplace before manifest, permissions, conformance, and signing
  rules exist.

## Diagram

See [system-overview.mmd](../assets/system-overview.mmd).

## Subsystem Map

| Subsystem | Owns | Does not own | Related ADRs | Current goal stage |
|---|---|---|---|---|
| `grcl-c` core contract | Public C ABI, opaque handles, result model, options structs, lifecycle declarations, storage, capability query declarations | Runtime behavior, backend implementation, SDK convenience APIs | [ADR-0002](../adr/ADR-0002-grcl-c-as-core-contract.md), [ADR-0005](../adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md), [ADR-0010](../adr/ADR-0010-grcl-c-capability-abi-representation.md) | G2-F closeout complete |
| Runtime capability and graph | Capability, availability, health, graph projection, negotiation schema, runtime participant model | Wire protocol, management transport, backend-specific discovery internals | [ADR-0004](../adr/ADR-0004-runtime-capability-exchange-and-scoped-graph.md), [ADR-0006](../adr/ADR-0006-runtime-capability-schema.md), [ADR-0010](../adr/ADR-0010-grcl-c-capability-abi-representation.md) | G1 schema artifacts complete; G2 ABI mapping complete |
| Backend SPI | Backend registration shape, lifecycle hooks, capability hooks, graph/diagnostics hooks, adapter containment | ROS2 public API design, runtime implementation, transport internals | [ADR-0008](../adr/ADR-0008-backend-spi-contract.md) | G3 design baseline complete; G4 implementation pending |
| MCU/RTOS profiles | Profile descriptors, subset rules, storage and executor constraints | Full desktop runtime requirements | [ADR-0007](../adr/ADR-0007-mcu-runtime-profile-contract.md) | G1 fixtures complete; runtime implementation pending |
| Language SDKs | C++, Python, and future SDK layering over `grcl-c` handles | Independent runtime semantics or backend-private type exposure | [ADR-0003](../adr/ADR-0003-language-sdks-over-grcl-c.md), [ADR-0009](../adr/ADR-0009-language-sdk-binding-contract.md) | G6 planning complete; implementation gated |
| Management plane | Future read-only snapshot contract, CLI/debug/dashboard data shape, diagnostics visibility | v1 auth, remote control, event streams, SDK permission enforcement | future G10 ADR | G10 deferred |
| Conformance | Header hygiene, schema fixtures, C ABI compile-only checks, local contract-drift gates, future CI-oriented gates | Runtime claims without corresponding runnable evidence | architecture decision docs only | G5 local harness complete |

## Design Traceability

| Design topic | Design home | Decision record | Concrete artifacts |
|---|---|---|---|
| Core C contract boundary | [GRCL-C API Shape](grcl-c-api-shape.md) | [ADR-0002](../adr/ADR-0002-grcl-c-as-core-contract.md), [ADR-0005](../adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md) | `src/grcl-c/include/grcl/c/*.h`, `src/grcl-c/tests/compile_headers_smoke.*` |
| Capability ABI representation | [GRCL-C Capability ABI Representation](grcl-c-capability-abi-representation.md) | [ADR-0010](../adr/ADR-0010-grcl-c-capability-abi-representation.md) | `capability.h`, `runtime.h`, `schemas/capability-negotiation-result.schema.yaml` |
| Runtime capability schema | [Runtime Capability Schema](runtime-capability-schema.md) | [ADR-0006](../adr/ADR-0006-runtime-capability-schema.md) | `schemas/runtime-capability-record.schema.yaml`, `tests/conformance/runtime-capability/*.yaml` |
| Runtime graph projection | [Runtime Capability Graph](runtime-capability-graph.md) | [ADR-0004](../adr/ADR-0004-runtime-capability-exchange-and-scoped-graph.md) | `docs/assets/runtime-graph.mmd`, capability graph fields |
| Backend boundary | [Backend SPI Contract](backend-spi-contract.md) | [ADR-0008](../adr/ADR-0008-backend-spi-contract.md) | G3 SPI ownership and function-table design; future G4 backend artifacts |
| MCU profile subsets | [MCU Runtime Profiles](mcu-runtime-profiles.md) | [ADR-0007](../adr/ADR-0007-mcu-runtime-profile-contract.md) | `schemas/mcu-profile.schema.yaml`, `tests/conformance/mcu-profiles/*.yaml` |
| SDK layering | [Language SDK Strategy](language-sdk-strategy.md) | [ADR-0003](../adr/ADR-0003-language-sdks-over-grcl-c.md), [ADR-0009](../adr/ADR-0009-language-sdk-binding-contract.md) | future G6 SDK wrapper skeletons |
| Management snapshot | [Management Plane Concept And Scope](management-plane-concept-and-scope.md) | future G10 ADR | future snapshot schema and tool output shape |

## Review Path

For a whole-middleware architecture review, read in this order:

1. [System Overview](system-overview.md) for architecture boundaries and subsystem relationships.
2. [Runtime Layering](runtime-layering.md) for layer rules and backend containment.
3. [GRCL-C API Shape](grcl-c-api-shape.md) for the current core public contract.
4. [Runtime Capability Schema](runtime-capability-schema.md) and [Runtime Capability Graph](runtime-capability-graph.md) for capability and topology semantics.
5. [Backend SPI Contract](backend-spi-contract.md) before entering G3 backend planning.
6. [Conformance And CI Strategy](conformance-and-ci-strategy.md) before claiming implementation stability.
7. [Middleware Goal Roadmap](../status/middleware-goal-roadmap.md) and [Goal Execution Queue](../status/goal-execution-queue.md) for current progress and gates.
