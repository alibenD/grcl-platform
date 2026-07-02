# ADR-0005 GRCL-C Handle And Lifecycle Contract

- Status: `draft`
- Date: `2026-07-02`

## Decision

The first `grcl-c` contract will use opaque handles, explicit create/destroy or init/fini
functions, options structs with `struct_size`, return-code errors, explicit allocator hooks, and
controlled-storage paths for constrained profiles.

## Required Handles

The initial handle set is documented in
[GRCL-C API Shape](../architecture/grcl-c-api-shape.md).

## Consequences

- Public headers must not expose C++ or ROS2 implementation types.
- Higher-level SDKs must map ownership to these handles.
- MCU/RTOS profiles can reject unsupported dynamic behavior before runtime start.

