# GRCL-C API Shape

## Purpose

This document defines the first public shape of the `grcl-c` contract before runtime and backend
behavior is implemented. It records the accepted header baseline and the constraints later
implementation goals must preserve.

This is the canonical design home for the `grcl-c` core contract. It owns the public C ABI shape,
the G2 closeout baseline, and the handoff constraints that later backend SPI and runtime
implementation goals must preserve.

## G2-F Closeout Baseline

G2-F closes `grcl-c` ABI Contract v0.1 as an implementation-ready contract baseline, not as a
runtime implementation. The accepted G2 baseline contains:

| Area | Artifact | Status |
|---|---|---|
| ABI versioning | `src/grcl-c/include/grcl/c/version.h` | v0.1 macros and packed ABI version present |
| Results | `src/grcl-c/include/grcl/c/result.h` | explicit result categories and result codes present |
| Opaque handles | `src/grcl-c/include/grcl/c/types.h` | runtime, node, endpoint, executor, backend, transport, allocator, storage handles declared |
| Controlled storage | `src/grcl-c/include/grcl/c/storage.h` | storage regions, flags, and bounded capacity descriptors present |
| Allocator policy | `src/grcl-c/include/grcl/c/allocator.h` | caller-provided allocator callbacks and limits present |
| Diagnostics | `src/grcl-c/include/grcl/c/diagnostics.h` | severity, category, scope, code, and record shape present |
| Runtime declarations | `src/grcl-c/include/grcl/c/runtime.h` | lifecycle, node, endpoint, executor, and capability query declarations present |
| Capability ABI | `src/grcl-c/include/grcl/c/capability.h` | fixed root summaries, request shape, negotiation result shape present |
| Compile-only checks | `src/grcl-c/tests/compile_headers_smoke.c`, `src/grcl-c/tests/compile_headers_smoke.cpp` | C11 and C++17 syntax-only smoke artifacts present |

G2-F does not freeze the final binary ABI for release. It freezes the v0.1 design baseline for G3
backend SPI design and minimum backend implementation planning.

## M1 Runnable-Core Closeout Baseline

M1 closes the first runnable `grcl-c` core path against the G2/G3 contract baseline. It does not
freeze a release ABI and does not authorize post-M1 feature work.

The M1 baseline contains:

| Area | Artifact | Status |
|---|---|---|
| Backend SPI materialization | `src/grcl-c/include/grcl/c/backend.h` | descriptor, runtime context, operation table, capability hooks, negotiation hook, and caller-buffer diagnostics hook compile in C and C++ |
| Core runtime lifecycle | `src/grcl-c/src/runtime.c` | create, init-with-storage, start, stop, and destroy run through a core-owned runtime handle |
| Bounded storage | `src/grcl-c/src/runtime.c` and `src/grcl-c/tests/runtime_lifecycle_test.c` | caller-provided runtime-object storage succeeds; missing or insufficient bounded storage fails with a resource-category result |
| Null/native-test backend | `src/grcl-runtime-native/src/null_backend.c` | private deterministic backend supplies lifecycle, capability, negotiation, diagnostics, and M3-D object-lifecycle no-op hook behavior |
| Native in-process backend selection | `grcl_runtime_options_t.profile_name = "native-inprocess"` | M3-E private selection convention; no public backend selector ABI yet |
| Capability query and negotiation | `src/grcl-c/tests/backend_capability_test.c` | deterministic capability record plus accepted, degraded accepted, and rejected incompatible negotiation cases pass |
| Diagnostics accessor | `src/grcl-c/include/grcl/c/runtime.h` and `src/grcl-c/tests/diagnostics_negative_state_test.c` | narrow public `grcl_runtime_get_diagnostics` caller-buffer API is present and verifies latest lifecycle or storage failure reporting |
| Local harness | `src/grcl-c/tests/run_m1_tests.sh` | C11/C++17 header smoke and runnable lifecycle/capability/diagnostics tests pass from workspace root, artifact-root override, and repository root |

M1 remains intentionally below publish/subscribe, service/client, executor scheduling, transport,
ROS2, MCU, SDK wrapper, management-plane, auth, remote-management, event-stream, Docker, CI,
repo-wide build-system, or external `grcl` migration scope.

## M3 Core Middleware Contract Baseline

M3 extends the `grcl-c` public contract from lifecycle/capability into local in-process core
middleware behavior. The first runnable M3 target is intentionally narrow: opaque bytes, executor
pull delivery, local runtime parameters, and C examples. It does not define network transport,
ROS2 projection, IDL/codegen, background threads, blocking waits, or SDK usability claims.

### Type Support Shape

M3 uses `grcl_type_support_t` as a small public identity record, not an IDL or serializer:

```c
typedef enum grcl_type_support_flags {
  GRCL_TYPE_SUPPORT_FLAG_NONE = 0x00000000u,
  GRCL_TYPE_SUPPORT_FLAG_FIXED_SIZE = 0x00000001u
} grcl_type_support_flags_t;

typedef struct grcl_type_support {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  uint64_t type_id;
  const char * type_name;
  size_t fixed_size;
  size_t alignment;
} grcl_type_support_t;
```

Rules:

- `type_id` plus topic or service name is the M3 compatibility key.
- `fixed_size` may be zero for variable-size byte payloads.
- M3 payload APIs copy opaque bytes. They do not serialize, deserialize, introspect, loan, or
  zero-copy transfer application objects.
- `grcl_type_support_t` is intentionally reclassified as a public ABI descriptor/value struct in
  M3, not an opaque handle. The complete struct belongs in `src/grcl-c/include/grcl/c/types.h` so
  applications and SDK wrappers can provide stack-allocated type identity records.

### Pub/Sub Shape

M3 adds bytes-oriented publish and take APIs:

```c
grcl_result_t grcl_publisher_publish_bytes(
  grcl_publisher_t * publisher,
  const void * payload,
  size_t payload_size);

grcl_result_t grcl_subscription_take_bytes(
  grcl_subscription_t * subscription,
  void * out_payload,
  size_t payload_capacity,
  size_t * out_payload_size);
```

Rules:

- A publisher and subscription match only when topic name and `type_id` match.
- `publish_bytes` copies caller bytes into backend-owned queue storage before returning.
- `spin_once` is the only M3 mechanism that progresses delivery from publisher queue to
  subscription take queue.
- `take_bytes` returns an explicit empty-queue result when no message is ready.
- `take_bytes` returns a capacity result without partial success when the caller buffer is too
  small.

### Service/Client Shape

M3 adds bytes-oriented request and response APIs:

```c
typedef uint64_t grcl_request_id_t;

grcl_result_t grcl_client_send_request_bytes(
  grcl_client_t * client,
  const void * request_payload,
  size_t request_payload_size,
  grcl_request_id_t * out_request_id);

grcl_result_t grcl_service_take_request_bytes(
  grcl_service_t * service,
  void * out_request_payload,
  size_t request_payload_capacity,
  size_t * out_request_payload_size,
  grcl_request_id_t * out_request_id);

grcl_result_t grcl_service_send_response_bytes(
  grcl_service_t * service,
  grcl_request_id_t request_id,
  const void * response_payload,
  size_t response_payload_size);

grcl_result_t grcl_client_take_response_bytes(
  grcl_client_t * client,
  grcl_request_id_t request_id,
  void * out_response_payload,
  size_t response_payload_capacity,
  size_t * out_response_payload_size);
```

Rules:

- Service and client compatibility uses service name plus request and response `type_id` values.
- `send_request_bytes` creates a monotonically increasing per-runtime request id.
- `spin_once` routes pending requests to services and pending responses to clients.
- M3 has no blocking call, timeout wait, future handle, remote service discovery, or cross-runtime
  transport.

### Executor Pull Shape

M3 uses the existing executor declarations as the only dispatch mechanism:

```c
grcl_result_t grcl_executor_create(
  grcl_runtime_t * runtime,
  const grcl_executor_options_t * options,
  grcl_executor_t ** executor);

grcl_result_t grcl_executor_add_node(
  grcl_executor_t * executor,
  grcl_node_t * node);

grcl_result_t grcl_executor_spin_once(
  grcl_executor_t * executor,
  uint64_t timeout_ns);
```

Rules:

- `timeout_ns` is accepted for API continuity, but M3 in-process native behavior is non-blocking.
- `spin_once` performs one bounded deterministic dispatch pass. It does not create threads.
- Executor membership is core-owned. The backend may observe member nodes through backend-private
  state but cannot create public node handles.

### Local Runtime Params Shape

M3 parameters are runtime-local. They are not node-scoped, distributed, watched, or synchronized.

```c
typedef enum grcl_param_type {
  GRCL_PARAM_TYPE_UNKNOWN = 0x0000,
  GRCL_PARAM_TYPE_BOOL = 0x0001,
  GRCL_PARAM_TYPE_INT64 = 0x0002,
  GRCL_PARAM_TYPE_UINT64 = 0x0003,
  GRCL_PARAM_TYPE_FLOAT64 = 0x0004,
  GRCL_PARAM_TYPE_STRING = 0x0005,
  GRCL_PARAM_TYPE_BYTES = 0x0006
} grcl_param_type_t;

typedef struct grcl_param_record {
  size_t struct_size;
  uint32_t abi_version;
  const char * name;
  grcl_param_type_t type;
  const void * value;
  size_t value_size;
} grcl_param_record_t;

grcl_result_t grcl_runtime_param_set(
  grcl_runtime_t * runtime,
  const grcl_param_record_t * param);

grcl_result_t grcl_runtime_param_get(
  grcl_runtime_t * runtime,
  const char * name,
  grcl_param_record_t * out_param,
  void * value_buffer,
  size_t value_buffer_capacity,
  size_t * out_value_size);

grcl_result_t grcl_runtime_param_list(
  grcl_runtime_t * runtime,
  char * out_names,
  size_t names_capacity,
  size_t * out_names_size,
  size_t * out_param_count);
```

Rules:

- Parameter names are runtime-local UTF-8 byte strings with no distributed namespace semantics.
- `set` copies parameter value bytes.
- `get` writes value bytes into caller storage and returns capacity errors for small buffers.
- `list` returns a newline-separated name list in M3 to avoid a new iterator ABI.
- M3 does not add parameter callbacks, watchers, descriptors, constraints, or distributed sync.

### M3 Result Mapping

M3 tests and implementation must use concrete `grcl_result_t` values for negative paths. If a
listed code is not present in `src/grcl-c/include/grcl/c/result.h`, M3-C must add it as an
append-only enum value before any behavior tests or implementation depend on it.

| Condition | Required result |
|---|---|
| null handle, null required pointer, zero-length required name, invalid option struct | `GRCL_ERROR_INVALID_ARGUMENT` |
| operation against stopped/destroyed runtime, destroyed object, wrong lifecycle state | `GRCL_ERROR_BAD_STATE` |
| older backend missing a required M3 SPI hook, backend reports unsupported local operation | `GRCL_ERROR_UNSUPPORTED_CAPABILITY` |
| subscription take with no ready message | `GRCL_ERROR_NO_DATA` |
| service take with no ready request | `GRCL_ERROR_NO_DATA` |
| client take with no ready response for the request id | `GRCL_ERROR_NO_DATA` |
| request id is unknown to the client/backend | `GRCL_ERROR_NOT_FOUND` |
| parameter name does not exist | `GRCL_ERROR_NOT_FOUND` |
| no matching service exists in the runtime | `GRCL_ERROR_PEER_UNAVAILABLE` |
| topic/service type id mismatch | `GRCL_ERROR_TYPE_MISMATCH` |
| payload exceeds runtime/backend max payload bytes | `GRCL_ERROR_PAYLOAD_TOO_LARGE` |
| caller output buffer is too small for message, response, parameter value, or parameter name list | `GRCL_ERROR_CAPACITY_EXCEEDED` |
| core object table, backend queue, parameter table, or backend value storage is full | `GRCL_ERROR_CAPACITY_EXCEEDED` |
| backend operation fails after validation succeeds | `GRCL_ERROR_BACKEND_FAILURE` |

M3-C is therefore authorized to modify `src/grcl-c/include/grcl/c/result.h` to add these
append-only result values:

```c
GRCL_ERROR_NOT_FOUND = 0x0103,
GRCL_ERROR_NO_DATA = 0x0404,
GRCL_ERROR_TYPE_MISMATCH = 0x0303,
GRCL_ERROR_PAYLOAD_TOO_LARGE = 0x0203
```

## Naming Rules

- Public C symbols use the `grcl_` prefix.
- Opaque handle types use `_t` suffix.
- Options structs use `_options_t` suffix.
- Result codes use `grcl_result_t`.
- Size-bearing options structs must begin with `size_t struct_size`.
- Version-bearing options structs must include `uint32_t abi_version` when the struct can cross a
  binary compatibility boundary.

## Initial Opaque Handles

| Handle | Purpose |
|---|---|
| `grcl_runtime_t` | runtime participant, lifecycle root, backend host |
| `grcl_node_t` | application node owned by a runtime |
| `grcl_endpoint_t` | graph-visible endpoint identity |
| `grcl_publisher_t` | typed publish endpoint |
| `grcl_subscription_t` | typed subscribe endpoint |
| `grcl_service_t` | service server endpoint |
| `grcl_client_t` | service client endpoint |
| `grcl_executor_t` | scheduler/executor profile surface |
| `grcl_backend_t` | backend implementation handle |
| `grcl_transport_t` | transport implementation handle |
| `grcl_allocator_t` | allocator policy handle |
| `grcl_storage_t` | caller-provided controlled storage descriptor |

`grcl_type_support_t` is not an opaque handle in M3. It is a public descriptor/value struct defined
in `types.h` and referenced by publisher, subscription, service, and client options.

## Result Model

`grcl_result_t` must separate permanent incompatibility from transient runtime state.

| Category | Example Codes |
|---|---|
| success | `GRCL_OK` |
| caller error | `GRCL_ERROR_INVALID_ARGUMENT`, `GRCL_ERROR_BAD_STATE` |
| resource error | `GRCL_ERROR_OUT_OF_MEMORY`, `GRCL_ERROR_CAPACITY_EXCEEDED`, `GRCL_ERROR_PAYLOAD_TOO_LARGE` |
| compatibility error | `GRCL_ERROR_UNSUPPORTED_CAPABILITY`, `GRCL_ERROR_INCOMPATIBLE_VERSION`, `GRCL_ERROR_TYPE_MISMATCH` |
| runtime error | `GRCL_ERROR_TIMEOUT`, `GRCL_ERROR_INTERRUPTED`, `GRCL_ERROR_BACKEND_FAILURE`, `GRCL_ERROR_NO_DATA` |
| graph/session error | `GRCL_ERROR_PEER_UNAVAILABLE`, `GRCL_ERROR_CHANNEL_REJECTED` |

Detailed diagnostics must be available through explicit diagnostic APIs rather than by expanding
every result code.

## Runtime Lifecycle Shape

```c
grcl_result_t grcl_runtime_create(
  const grcl_runtime_options_t * options,
  grcl_runtime_t ** runtime);

grcl_result_t grcl_runtime_init_with_storage(
  const grcl_runtime_options_t * options,
  const grcl_storage_t * storage,
  grcl_runtime_t ** runtime);

grcl_result_t grcl_runtime_start(grcl_runtime_t * runtime);
grcl_result_t grcl_runtime_stop(grcl_runtime_t * runtime);
grcl_result_t grcl_runtime_destroy(grcl_runtime_t * runtime);
```

Rules:

- `create` may allocate through the configured allocator.
- `init_with_storage` must not perform hidden heap allocation after successful initialization for
  profiles that declare `no_heap_after_init`.
- `start` must fail if required backend capabilities are missing.
- `destroy` must release owned nodes, endpoints, sessions, and backend state in a deterministic
  order defined by the lifecycle contract.

## Node And Endpoint Shape

```c
grcl_result_t grcl_node_create(
  grcl_runtime_t * runtime,
  const grcl_node_options_t * options,
  grcl_node_t ** node);

grcl_result_t grcl_node_destroy(grcl_node_t * node);

grcl_result_t grcl_publisher_create(
  grcl_node_t * node,
  const grcl_publisher_options_t * options,
  grcl_publisher_t ** publisher);

grcl_result_t grcl_subscription_create(
  grcl_node_t * node,
  const grcl_subscription_options_t * options,
  grcl_subscription_t ** subscription);
```

Rules:

- Nodes are owned by exactly one runtime.
- Endpoints are owned by exactly one node.
- Endpoint creation must validate local runtime profile constraints before advertising remote graph
  state.
- Endpoint matching across runtimes must pass through runtime capability negotiation.

## Capability Query Shape

```c
grcl_result_t grcl_runtime_get_capabilities(
  const grcl_runtime_t * runtime,
  grcl_runtime_capability_record_t * out_record);

grcl_result_t grcl_runtime_negotiate_capabilities(
  const grcl_runtime_t * runtime,
  const grcl_runtime_capability_request_t * request,
  grcl_capability_negotiation_result_t * out_result);

grcl_result_t grcl_runtime_get_diagnostics(
  const grcl_runtime_t * runtime,
  grcl_diagnostic_record_t * out_records,
  size_t record_capacity,
  size_t * out_record_count);
```

Rules:

- Capability queries must not report current resource availability as static capability.
- Capability records use the selected hybrid public C ABI strategy: fixed root structs for stable
  identity, protocol, profile/class identifiers, storage and entity limits, lifecycle/result
  summaries, and graph projection summaries.
- M3 runtime-local parameter support is represented through append-only capability fields and a
  summary flag added in M3-C. The exact fields are:
  `GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS`, `max_parameters`,
  `parameter_name_buffer_bytes`, and `parameter_value_buffer_bytes`.
- Variable capability families such as transports, QoS variants/subsets, encodings, diagnostics
  capabilities, and security capability subsets must use descriptor structs, iterator-style APIs, or
  caller-provided output buffer APIs instead of a giant flattened public root struct.
- Opaque serialized blob support is deferred from G2 and may be revisited only as a future protocol
  or management-plane extension point.
- Negotiation must return `accepted`, `degraded_accepted`, or `rejected_incompatible`.
- Silent downgrade is not allowed.
- Capability, availability, and health remain separate concepts and must not be collapsed into one
  query result object.

See [GRCL-C Capability ABI Representation](grcl-c-capability-abi-representation.md) and
[ADR-0010 GRCL-C Capability ABI Representation](../adr/ADR-0010-grcl-c-capability-abi-representation.md).

## Controlled Storage Shape

`grcl_storage_t` is a caller-provided description of storage regions and capacities.

Required storage dimensions:

- runtime object bytes
- node table capacity
- endpoint table capacity
- session table capacity
- channel table capacity
- message buffer bytes
- graph cache bytes
- diagnostics buffer bytes
- executor state bytes

Profiles may set some capacities to zero when the capability is unsupported.

## ABI Compatibility Rules

- Public structs crossing ABI boundaries must include `struct_size`.
- Optional trailing fields may be added only when older callers can safely pass a smaller
  `struct_size`.
- Public enums must reserve explicit unknown/invalid values.
- Public C headers must not expose C++ types, exceptions, templates, RTTI, `rcl`, `rmw`, or
  `rclcpp` types.

## Backend SPI Handoff

G3 backend SPI design must preserve the following `grcl-c` baseline constraints:

- `grcl-c` remains the public semantic boundary. Backend SPI types are internal contract artifacts
  for implementations and must not leak backend-private ROS2, native, MCU, or simulator types into
  public SDK headers.
- Runtime lifecycle ownership starts in the core contract. A backend may allocate backend-private
  state through the provided allocator/storage policy, but public runtime, node, endpoint, and
  executor handles remain core-owned opaque handles.
- Backend lifecycle hooks must map to the public lifecycle declarations in this document:
  configure or init, start, stop, destroy. G3 must not add public lifecycle states that are not
  representable through `grcl_result_t`, diagnostics, and capability negotiation.
- Backend capability hooks must fill or negotiate the existing capability root summaries. G3 may
  define internal descriptor-family APIs, but it must not require a new G2 schema or an opaque
  serialized blob to make the first native/null backend testable.
- The completed M1 minimum backend tests target lifecycle, bounded storage, capability query,
  capability negotiation, and diagnostics only. M3 is the first approved goal that may implement
  local in-process publish/subscribe, service/client, executor pull, and runtime-local params.

## M1 Minimum Runnable Target

The first native/null backend has now proved only this contract path:

1. Create a runtime with `grcl_runtime_create` or `grcl_runtime_init_with_storage`.
2. Start and stop the runtime through the public lifecycle declarations.
3. Query capabilities through `grcl_runtime_get_capabilities`.
4. Negotiate a compatible and an incompatible capability request.
5. Destroy the runtime deterministically.

The minimum target includes negative cases for invalid arguments, incompatible capability, bad
state, and bounded storage or capacity failure. It must not be interpreted as permission to open
channels, publish messages, spin executor behavior beyond a compile/runtime placeholder, or project
ROS2 graph state.
