# grcl-py

`grcl-py` is the Python SDK layer over `grcl-c`.

It must not define independent runtime lifecycle, ownership, capability, or backend semantics.
Its role is to provide a Python-facing API over `grcl-c`, potentially through a private native shim,
while preserving `grcl-c` as the semantic source of truth.

No SDK behavior is implemented in this skeleton phase.
