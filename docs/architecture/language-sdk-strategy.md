# Language SDK Strategy

## Purpose

Language SDKs provide developer-friendly APIs while preserving one shared `grcl-c` semantic
contract.

## Required Layering

```text
grcl-cpp -> grcl-c
grcl-py  -> private native shim -> grcl-c
grcl-rs  -> FFI wrapper -> grcl-c
C apps   -> grcl-c directly
```

## C++ SDK

Responsibilities:

- RAII ownership over `grcl-c` handles
- typed publisher/subscription wrappers
- template-based type safety
- allocator integration
- optional exception facade over return-code APIs
- no independent runtime lifecycle semantics

Forbidden:

- independent core runtime implementation
- public API behavior that cannot map to `grcl-c`
- exposing ROS2 backend types through GRCL public C++ SDK

## Python SDK

Responsibilities:

- Pythonic API over `grcl-c`
- private native extension or C FFI layer
- explicit lifetime mapping for runtime, node, endpoint, and executor objects
- wheel/package strategy after C ABI shape stabilizes

Forbidden:

- binding the public `grcl-cpp` SDK as the long-term semantic source
- silently changing lifecycle, ownership, QoS, or capability behavior

## Rust SDK

Responsibilities:

- safe wrapper over `grcl-c` FFI
- ownership and lifetime enforcement in Rust types
- no independent runtime semantics

## Cross-Language Conformance

Initial conformance scenarios:

- runtime create/start/stop/destroy
- node create/destroy
- endpoint capability rejection
- publish/subscribe basic data path
- capability negotiation accepted/degraded/rejected results
- controlled-storage profile behavior where applicable

Each language SDK must pass conformance against the same core behavior before being called stable.

