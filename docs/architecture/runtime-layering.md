# Runtime Layering

## Layer Model

```text
Language SDK Layer
  grcl-cpp / grcl-py / grcl-rs / direct grcl-c

Core Contract Layer
  grcl-c handles, options, lifecycle, errors, type support, capability query

Runtime Host Layer
  runtime participant, node ownership, endpoint ownership, executor integration

Backend SPI Layer
  native, ROS2 adapter, MCU/RTOS, gateway, simulator

Transport and Platform Layer
  sockets, shared memory, DDS/ROS2, UART, CAN-FD, RTOS tasks, clocks, allocators
```

## Rules

- Public SDKs must not expose backend-private implementation types.
- Backend implementations may optimize internally but must preserve `grcl-c` observable behavior.
- MCU/RTOS backends implement declared profile subsets.
- Cross-runtime communication must start with runtime identity and capability exchange.
- Tools and management APIs must observe runtime graph state, node graph state, and negotiation
  results.

## Baseline Decisions

- The initial `grcl-c` handle set is defined in [GRCL-C API Shape](grcl-c-api-shape.md).
- Runtime lifecycle starts with create/init-with-storage, start, stop, and destroy.
- Backend responsibilities are defined in [Backend SPI Contract](backend-spi-contract.md).
- Runtime capability, availability, health, graph projection, and negotiation result schemas are
  defined in [Runtime Capability Schema](runtime-capability-schema.md).
- MCU/RTOS executor profile constraints are defined through profile descriptors in
  [MCU Profile Schema](mcu-profile-schema.md).

## Remaining Design Gates

- Management API transport and authentication remain design gates for a later management-plane ADR.
- Final executor scheduling semantics require implementation feedback from `baremetal-min`,
  `rtos-basic`, and `linux-full` skeletons.
