# grcl-cpp

`grcl-cpp` is the C++ SDK layer over `grcl-c`.

It must not define independent runtime lifecycle, ownership, capability, or backend semantics.
Its role is to provide idiomatic C++ ownership and type-safe wrappers that preserve the `grcl-c`
contract as the semantic source of truth.

No SDK behavior is implemented in this skeleton phase.
