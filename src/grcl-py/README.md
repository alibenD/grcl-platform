# grcl-py

`grcl-py` is the Python SDK layer over `grcl-c`.

It must not define independent runtime lifecycle, ownership, capability, or backend semantics.
Its role is to provide a Python-facing API over `grcl-c`, through a private native boundary that
mirrors the `grcl-c` runtime calls while preserving `grcl-c` as the semantic source of truth.

This G6-D skeleton only covers ownership and explicit result mapping:

- `grcl_py.result` mirrors accepted `grcl_result_t` values.
- `grcl_py.runtime.Runtime` owns one native runtime handle and prevents double close.
- context-manager entry and exit map to lifecycle cleanup.
- real shared-library loading, wheels, and package-manager policy remain deferred.

No backend, graph, transport, ROS2, MCU, simulator, management-plane, auth, or remote-management
behavior is implemented in this skeleton phase.
