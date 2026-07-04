# grcl-c public headers

This directory is reserved for the future public C headers that define the `grcl-c` contract.

The current header surface covers runtime lifecycle declarations, ownership types, controlled
storage, result and error handling, diagnostics, capability query and negotiation structs, and the
backend SPI host boundary.

`backend.h` is a backend-implementer SPI contract. It declares ABI-sized backend descriptors,
runtime creation context, backend-owned runtime state, lifecycle hooks, capability hooks, and
caller-buffer diagnostics retrieval. It does not implement backend registration, runtime behavior,
transport, graph projection, ROS2, MCU, simulator, SDK, or management-plane behavior.

No runtime behavior is implemented in this skeleton phase.
