# GRCL-C Contract Boundary

## Purpose

`grcl-c` is the semantic center of the platform. It provides the stable C-compatible boundary used
by platform runtimes, backend SPI implementations, language SDKs, tooling, and constrained
MCU/RTOS targets.

## Initial Contract Areas

- Runtime lifecycle and identity.
- Node and endpoint ownership.
- Publisher/subscription/service/client object handles.
- Executor or scheduler-facing hooks.
- Allocator and controlled storage.
- Error/result model.
- Capability query and negotiation.
- Type support and IDL/codegen integration.
- Backend SPI registration and dispatch.

## Required API Style

- opaque handles
- `create` / `destroy`
- options structs with size/version fields
- return-code error model
- explicit allocator hooks
- explicit controlled-storage path for MCU/RTOS profiles
- no implicit C++ exception or RTTI dependency

## Controlled Storage Requirement

MCU/RTOS profiles must be able to initialize core objects through preallocated storage or equivalent
capacity-checked paths. Heap allocation after initialization must be forbidden for hard or
near-real-time profiles that declare this constraint.

## Binding Policy

- `grcl-cpp` wraps `grcl-c`.
- `grcl-py` binds `grcl-c` directly or through a private shim.
- Future Rust or other SDKs must bind the shared core contract.

## Baseline Decisions

- Public C symbols use the `grcl_` prefix.
- Opaque handle types use the `_t` suffix.
- Options structs use the `_options_t` suffix.
- Public structs crossing ABI boundaries include `struct_size`.
- Version-bearing structs include `abi_version` when they can cross a binary compatibility
  boundary.
- Runtime capability query and negotiation are explicit `grcl-c` API surfaces.
- `grcl_storage_t` describes caller-provided storage regions and capacities.

The detailed first API shape is defined in [GRCL-C API Shape](grcl-c-api-shape.md).

## Implementation Gates

Before writing public headers, the implementation plan must map each header to the API shape
document and verify that no C++ or ROS2 types leak into public C headers.

Type support ABI details are allowed to start as opaque type identity and generated-operation hooks
in the first skeleton phase. Final serialization and codegen ABI requires a later IDL-specific ADR.
