# ADR-0002 GRCL-C As Core Contract

- Status: `accepted`
- Date: `2026-07-02`

## Decision

`grcl-c` is the core semantic contract for the GRCL platform. It is not a thin wrapper over
`grcl-cpp`.

`grcl-c` owns the stable contract for:

- runtime lifecycle
- node and endpoint ownership
- backend SPI hosting
- allocator and controlled storage
- result and error categories
- capability query and negotiation
- type support entry points
- C-compatible ABI boundaries for higher-level SDKs

## Rationale

GRCL must support MCU/RTOS targets, Linux runtimes, ROS2 adapters, native backends, gateways, and
language SDKs. A C-compatible core contract is the lowest common denominator that can support
embedded targets and language bindings without forcing C++ ABI or runtime semantics into every
deployment.

## Consequences

- `grcl-cpp` is an idiomatic C++ SDK over `grcl-c`.
- `grcl-py` binds `grcl-c` directly or through a private native shim over `grcl-c`.
- MCU/RTOS deployments can use `grcl-c` profile subsets directly.
- Any future C++ internal optimization must preserve the observable `grcl-c` contract.

