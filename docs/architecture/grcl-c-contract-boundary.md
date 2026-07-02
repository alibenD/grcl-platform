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

## Open Design Items

- Exact naming scheme for handles and options.
- ABI versioning strategy.
- Capability query function shape.
- Type support ABI.
- Controlled-storage object layout constraints.

