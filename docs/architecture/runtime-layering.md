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

## Open Design Items

- Exact `grcl-c` handle set.
- Runtime lifecycle state machine.
- Backend SPI method set.
- Capability record schema.
- Executor profile mapping for MCU/RTOS.
- Management API transport and authentication.

