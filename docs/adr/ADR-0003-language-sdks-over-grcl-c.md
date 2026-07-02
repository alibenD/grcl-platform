# ADR-0003 Language SDKs Over GRCL-C

- Status: `accepted`
- Date: `2026-07-02`

## Decision

Language SDKs are wrappers over the shared `grcl-c` contract.

The intended layering is:

```text
Application C++   -> grcl-cpp -> grcl-c
Application Python -> grcl-py  -> private native shim -> grcl-c
Application C      -> grcl-c
Future Rust SDK    -> grcl-rs  -> grcl-c FFI
```

`grcl-py` must not bind the public `grcl-cpp` API as its long-term semantic source.

## Rationale

Keeping language SDKs on top of `grcl-c` avoids duplicated lifecycle, ownership, error, QoS,
capability, and backend semantics. It also avoids coupling Python and future SDKs to C++ public ABI
details.

## Consequences

- Language-specific SDKs can provide idiomatic APIs without changing core behavior.
- Cross-language conformance tests must compare behavior through the shared core contract.
- The current `github.com/alibenD/grcl` C++ package should be treated as a `grcl-cpp` candidate,
  not as the core runtime contract.

