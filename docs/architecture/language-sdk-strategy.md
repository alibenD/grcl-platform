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

## G6 M2 Boundary Baseline

G6 plans M2 Cross-Language SDK Boundary Baseline. The baseline is intentionally narrower than a
usable application SDK: it proves that language wrappers can own and release `grcl-c` handles
without defining independent lifecycle, result, capability, negotiation, diagnostics, backend, or
runtime semantics.

The G6 boundary covers only the M1-proven surface:

- runtime ownership over `grcl_runtime_t`.
- create or init-with-storage, start, stop, and destroy lifecycle forwarding.
- explicit `grcl_result_t` mapping.
- capability query and negotiation forwarding.
- diagnostics retrieval forwarding.
- local conformance checks that detect wrapper drift away from `grcl-c`.

G6 does not cover publish/subscribe, service/client, executor scheduling, transport, graph runtime,
ROS2 adapter behavior, simulator behavior, MCU runtime behavior, management-plane behavior, auth,
remote management, event streams, package distribution, CI, Docker, or external `grcl` migration.

The selected G6 Python binding substrate is Option A: a private `ctypes` or dynamic-library shim
boundary with package distribution deferred. Implementation must stop if real shared-library
production, native extension framework selection, or package policy becomes required.

## C++ SDK

Responsibilities:

- RAII ownership over `grcl-c` handles
- typed publisher/subscription wrappers
- template-based type safety
- allocator integration
- optional exception facade over return-code APIs
- no independent runtime lifecycle semantics

G6 `grcl-cpp` skeleton requirements:

- `Runtime` owns one `grcl_runtime_t *`.
- copy operations are deleted.
- move operations transfer ownership.
- destruction maps to `grcl_runtime_destroy`.
- lifecycle methods forward to `grcl_runtime_start` and `grcl_runtime_stop`.
- result wrappers preserve `grcl_result_t` without adding incompatible categories.
- compile-only or smoke verification must use local artifact-root output, not a repo-wide build
  system.

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

G6 `grcl-py` skeleton requirements:

- Python wrapper objects own a private native handle reference and prevent double close.
- context-manager behavior maps to explicit `grcl-c` lifecycle ownership.
- `_native` remains a private boundary shaped like `grcl-c`, not a backend or `grcl-cpp` binding.
- standard-library tests may use a fake native boundary for ownership semantics until a separate
  build/package decision authorizes real shared-library loading.
- package distribution, wheels, extension modules, and package-manager policy are deferred from G6
  unless explicitly approved.

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

For G6, cross-language conformance is limited to wrapper boundary checks and M1-surface lifecycle
smoke coverage. Stable SDK claims remain deferred until later goals add data-path, graph,
transport, and release conformance.
