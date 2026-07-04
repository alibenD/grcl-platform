# grcl-c tests

This directory holds the local M1 smoke and lifecycle tests for the public `grcl-c` header set and
the first minimal core runtime implementation.

Durable local M1 command:

```bash
src/grcl-platform/src/grcl-c/tests/run_m1_tests.sh
```

The harness may be run from either the workspace root or the repository root. It resolves the
repository root from the script path, writes generated compiler outputs under the workspace-local
artifact tree by default, and supports an explicit override:

```bash
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-m1-artifacts \
  src/grcl-platform/src/grcl-c/tests/run_m1_tests.sh
```

Default artifact root in the expected workspace layout:

```text
/Users/aliben/Project/grcl-platform_ws/artifacts
```

If the script cannot resolve that expected workspace layout and
`GRCL_PLATFORM_ARTIFACT_ROOT` is unset, it fails with a clear error instead of writing generated
files under the repository.

Current harness scope for M1 closeout:

- compile `compile_headers_smoke.c` as C11
- compile `compile_headers_smoke.cpp` as C++17
- run the resulting smoke binaries from the artifact tree
- compile `src/grcl-c/src/runtime.c` and `src/grcl-runtime-native/src/null_backend.c` with
  `runtime_lifecycle_test.c` as C11
- run the lifecycle test binary from the artifact tree
- compile `src/grcl-c/src/runtime.c` and `src/grcl-runtime-native/src/null_backend.c` with
  `backend_capability_test.c` as C11
- run the backend capability test binary from the artifact tree
- compile `src/grcl-c/src/runtime.c` and `src/grcl-runtime-native/src/null_backend.c` with
  `diagnostics_negative_state_test.c` as C11
- run the diagnostics negative-state test binary from the artifact tree
- keep generated outputs under `artifacts/m1/grcl-c/` instead of inside the repository

M1 is closed at this harness scope. Post-M1 test expansion requires a new user-approved goal window
and must continue to keep generated outputs outside the repository.

G5 adds a local C ABI/header conformance entrypoint:

```bash
python3 scripts/check-c-abi.py
```

The G5 checker compiles the two header smoke sources to object files only, scans public
`grcl-c` headers for forbidden C++/ROS/build/runtime leakage, and verifies required
ABI-boundary structs retain `struct_size` and `abi_version`. It writes compiler outputs and its
report under `artifacts/g5/c-abi/` by default, or under
`$GRCL_PLATFORM_ARTIFACT_ROOT/g5/c-abi/` when the variable is set.

The smoke sources exist to prove that the current header surface:

- compiles as C11
- compiles as C++17
- can be included as one public header set without leaking runtime implementation, backend
  implementation, or SDK behavior
- preserves representative ABI surface references through compile-time-only assertions
- includes the backend SPI descriptor, runtime context, operation table, family ids, and
  caller-buffer diagnostics hook without adding runtime behavior

Current artifacts:

- `compile_headers_smoke.c`: C11 syntax-only smoke for the public headers
- `compile_headers_smoke.cpp`: C++17 syntax-only smoke for the public headers
- `runtime_lifecycle_test.c`: plain-C lifecycle test for create, storage-backed initialization,
  start, stop, destroy, repeated start/stop, missing bounded storage, and bounded storage missing a
  runtime-object region
- `backend_capability_test.c`: plain-C backend capability and negotiation test for the
  null/native-test backend, including deterministic identity/profile/version/schema fields,
  runtime-only graph projection, required summary flags, accepted negotiation, degraded accepted
  negotiation for unsupported optional preferences, rejected incompatible negotiation for
  unsupported required fields, and invalid capability query arguments
- `diagnostics_negative_state_test.c`: plain-C diagnostics and negative-state test for the public
  `grcl_runtime_get_diagnostics` caller-buffer API, invalid diagnostics arguments, diagnostics
  count query, repeated start/stop bad-state diagnostics, incompatible capability negotiation, and
  missing bounded storage

The smoke sources are intentionally inert. They do not call runtime or backend functions, allocate
memory, perform I/O, or depend on a test framework. Their job is to keep the `grcl-c` public ABI and
backend SPI headers compilable.

The lifecycle test is also framework-free. Its positive storage path supplies a bounded
`GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT` region and expects `grcl_runtime_init_with_storage` to
place the private runtime handle in that caller-provided region rather than allocating from the
heap. It deliberately does not test native/null backend capability behavior, diagnostics,
publish/subscribe, service/client, executor scheduling, transport, ROS2, SDK, management, auth, or
remote/event stream behavior.

The backend capability test is also framework-free. It exercises only the M1-E default
null/native-test backend path through public runtime lifecycle, capability query, and capability
negotiation calls. It deliberately does not test publish/subscribe, service/client, executor
scheduling, transport, ROS2 graph projection, SDK behavior, management snapshots, auth, descriptor
iterators, or remote/event stream behavior.

The diagnostics negative-state test is also framework-free. It exercises only the M1-F public
runtime diagnostics accessor and already-supported negative states. It deliberately does not add
diagnostic schemas, descriptor iterators, serialized blobs, management snapshots, health streams,
remote events, or broader runtime behavior.
