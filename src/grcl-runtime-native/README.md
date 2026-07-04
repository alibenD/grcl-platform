# grcl-runtime-native

`grcl-runtime-native` is the planned Linux/native GRCL runtime and backend family.

Its responsibility is to host native runtime lifecycle, graph state, capability reporting,
communication, scheduling hooks, memory policy, diagnostics, and related backend SPI integration
behind the `grcl-c` core contract boundary.

M1 adds a private `src/null_backend.c` null/native-test backend used by the local `grcl-c` M1 test
harness. It is not a production native transport implementation and does not expose a public
backend registration API.

The M1 null/native-test backend:

- implements the existing `grcl_backend_ops_t` lifecycle, capability, negotiation, and diagnostic
  hook slots.
- allocates only backend-private test state with host allocation.
- reports deterministic capability root-summary values for runtime identity, implementation
  version, protocol version, capability schema version, runtime-only graph projection, bounded
  storage, bounded capacities, and deterministic destroy.
- returns accepted, degraded accepted with explicit downgrade evidence, or rejected incompatible
  negotiation results using only the fixed fields in `capability.h`.
- returns zero descriptor counts for transport, QoS, encoding, diagnostics, and security families.

Transport, publish/subscribe, service/client, executor scheduling, ROS2 behavior, SDK wrappers,
management plane behavior, auth, remote/event stream behavior, descriptor iterators, Docker, CI,
and external `grcl` migration are intentionally not implemented here.
