# ADR-0009 Language SDK Binding Contract

- Status: `draft`
- Date: `2026-07-02`

## Decision

Language SDKs bind the shared `grcl-c` contract. C++ provides an idiomatic wrapper; Python binds
`grcl-c` directly or through a private native shim; future Rust wraps `grcl-c` through FFI.

## Rationale

This prevents lifecycle, ownership, error, QoS, graph, and capability semantics from diverging
between languages.

## Consequences

- `grcl-py` must not use the public `grcl-cpp` SDK as its semantic source.
- Cross-language conformance tests are required before stable SDK claims.
- C++ convenience features must remain representable through `grcl-c`.

