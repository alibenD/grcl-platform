# grcl-cpp

`grcl-cpp` is the C++ SDK layer over `grcl-c`.

It must not define independent runtime lifecycle, ownership, capability, or backend semantics.
Its role is to provide idiomatic C++ ownership and type-safe wrappers that preserve the `grcl-c`
contract as the semantic source of truth.

The current skeleton provides:

- `grcl::Result` as a direct alias of `grcl_result_t`
- `grcl::Runtime` as a minimal RAII wrapper over one `grcl_runtime_t *`
- start/stop/destroy forwarding without introducing new runtime semantics
- a local smoke test and compile script that build directly against the existing `grcl-c`
  implementation

No SDK behavior beyond this ownership boundary is implemented in this skeleton phase.
