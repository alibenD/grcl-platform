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

G6 completes the M2 Cross-Language SDK Boundary Baseline. The baseline is intentionally narrower
than a usable application SDK: it proves that language wrappers can own and release `grcl-c`
handles without defining independent lifecycle, result, capability, negotiation, diagnostics,
backend, or runtime semantics.

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

## M5 Local Core SDK Completion Target

M5 is the next approved implementation goal after G6, M3, and M4. It upgrades `grcl-cpp` from a
boundary skeleton into the first feature-complete language wrapper for the approved local-core GRCL
surface.

M5 `grcl-cpp` target surface:

- runtime lifecycle and ownership
- node ownership
- executor membership and `spin_once` forwarding
- publisher and subscription lifecycle plus local bytes data path
- service and client lifecycle plus spin-driven request/reply
- runtime-local parameter set/get/list forwarding
- capability query, negotiation, and diagnostics forwarding where the C mapping is direct

M5 does not change the required layering:

```text
grcl-cpp -> grcl-c
```

The wrapper may improve ownership ergonomics and type safety, but it must not introduce
independent lifecycle states, transport behavior, backend objects, scheduling policy, or blocking
call semantics. If a desired C++ API cannot be explained as a direct wrapper over `grcl-c`, M5
must stop and return to the C design homes first.

### M5 GRCL-CPP Wrapper Families

M5 `grcl-cpp` is required to cover the full approved local-core C surface. The wrapper families are:

| Wrapper family | Required responsibility | Source `grcl-c` surface |
|---|---|---|
| `Result` | direct result-code mapping with no incompatible error taxonomy | `grcl_result_t` |
| `Runtime` | lifecycle ownership, capability query, negotiation forwarding, diagnostics forwarding, runtime-local params forwarding | `grcl_runtime_*`, `grcl_runtime_param_*` |
| `Node` | move-only node ownership and runtime association | `grcl_node_*` |
| `Executor` | move-only executor ownership, node membership, bounded `spin_once` forwarding | `grcl_executor_*` |
| `Publisher` | move-only publisher ownership plus bytes publish forwarding | `grcl_publisher_*` |
| `Subscription` | move-only subscription ownership plus bytes take forwarding | `grcl_subscription_*` |
| `Service` | move-only service ownership plus request take and response send forwarding | `grcl_service_*` |
| `Client` | move-only client ownership plus request send and response take forwarding | `grcl_client_*` |
| runtime metadata forwarding | direct capability query, capability negotiation, and diagnostics retrieval forwarding without policy rewrite | `grcl_runtime_get_capabilities`, `grcl_runtime_negotiate_capabilities`, `grcl_runtime_get_diagnostics` |
| lightweight type-support adapter | non-owning or value-style access to `grcl_type_support_t` identity data where needed for construction | `grcl_type_support_t` |

M5 does not require a second semantic layer above those wrappers. In particular, M5 does not
require templates, codegen, IDL integration, serialization helpers, futures, blocking service
calls, graph observers, or a package distribution story.

### M5 GRCL-CPP Surface Rules

- Public C++ wrappers remain move-only by default unless shared ownership is explicitly proven
  necessary by the `grcl-c` contract. M5 assumes direct ownership, not reference-counted ownership.
- Public methods should mirror `grcl-c` behavior closely enough that test cases can be translated
  between C and C++ without changing semantic expectations.
- Bytes-oriented methods remain acceptable in C++ for M5. Convenience overloads are allowed only
  when they preserve the same copy/capacity semantics as the underlying C calls.
- Runtime-local params remain runtime-local in C++ as well. `grcl-cpp` must not invent node-scoped
  or distributed param semantics.
- Capability query, negotiation, and diagnostics may be wrapped as direct forwarding methods or
  helper records, but they must continue to expose the underlying `grcl-c` meaning rather than a
  rewritten policy layer.

### M5 GRCL-CPP Acceptance Matrix

`grcl-cpp` may be called complete for M5 only when all of the following exist:

| Acceptance area | Required evidence |
|---|---|
| runtime wrapper baseline | C++ tests covering create/init/start/stop/destroy, move-only ownership, and storage-backed lifecycle |
| capability/negotiation/diagnostics forwarding | targeted C++ runtime-wrapper tests covering capability query, negotiation result forwarding, diagnostics retrieval, and representative negative-path semantics |
| node and executor wrappers | C++ tests covering node ownership, executor membership, and bounded `spin_once` forwarding |
| pub/sub wrappers | C++ tests plus a runnable pub/sub example |
| service/client wrappers | C++ tests plus a runnable service/client example |
| runtime-local params wrappers | C++ tests plus a runnable params example |
| combined wrapper flow | a runnable combined core middleware C++ example |
| conformance integration | top-level local conformance includes the approved C++ wrapper/example stage |

For M5, "full wrapper" means full coverage of the approved native-backend local-core C surface. It
does not mean stable package distribution, broad ergonomic sugar, async APIs, or cross-backend SDK
coverage.

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

Current G6 implementation evidence:

- `src/grcl-cpp/include/grcl/cpp/result.hpp`
- `src/grcl-cpp/include/grcl/cpp/runtime.hpp`
- `src/grcl-cpp/tests/runtime_wrapper_smoke.cpp`
- `src/grcl-cpp/tests/run_g6_cpp_tests.sh`

This evidence proves only the minimal runtime ownership boundary, not full `grcl-cpp` parity with
the approved local-core C surface.

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

## G6 Implementation Closeout

G6 produced:

- a minimal `grcl-cpp` RAII wrapper skeleton over `grcl_runtime_t`.
- a minimal `grcl-py` ownership wrapper skeleton over a private native boundary shaped like
  `grcl-c`.
- local C++ and Python smoke tests for the wrapper ownership boundary.
- `scripts/check-sdk-boundaries.py`, integrated into `scripts/run-conformance.sh`, to catch local
  SDK boundary drift.

G6 remains a boundary baseline, not a stable SDK release. Package distribution, shared-library
production, native extension framework selection, data-path APIs, graph behavior, transport, ROS2,
MCU runtime, simulator, management-plane behavior, auth, remote management, event streams, CI,
Docker, and external `grcl` migration remain deferred to later goal-specific plans.

M5 is the first goal allowed to expand `grcl-cpp` beyond that boundary baseline, but only for the
already approved native-backend local-core C surface. `grcl-py` remains at the G6 boundary stage
until a separate goal authorizes broader SDK expansion.
