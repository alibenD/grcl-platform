# Backend SPI Contract

## Purpose

This document is the canonical design home for the backend SPI. It defines the internal adapter
boundary that backend implementations must satisfy so the `grcl-c` core contract can host native,
ROS2, MCU/RTOS, gateway, and simulator runtimes without allowing any one backend to define public
GRCL semantics by accident.

G3 produced the first SPI shape as a design baseline. M1 materialized lifecycle, capability, and
diagnostics hooks through a private null/native-test backend. M3 extends the SPI design to local
in-process messaging while preserving backend containment. M5 keeps that containment and uses the
current native in-process backend as the local-core product baseline for `grcl-c` and `grcl-cpp`
completion.

## Backend Families

| Backend | Role | First implementation goal |
|---|---|---|
| null/native-test | deterministic local backend used to validate lifecycle, capability, diagnostics, and M3-D object lifecycle hooks without routing | completed in M1 minimum runnable-core scope; extended in M3-D with object-lifecycle no-op hooks only |
| native | in-process native routing for M3, later Linux/native transport work | M3 for in-process routing only |
| ros2 | ROS2 adapter and GRCL graph projection over ROS2 behavior | G8 |
| mcu | profile-limited bare-metal/RTOS runtime | G9 |
| gateway | downstream runtime representation and capability translation | G9/G10 input |
| simulator | deterministic graph, runtime, diagnostics, and snapshot validation | G7 |

## SPI Ownership Model

The core owns public objects. The backend owns backend-private state.

| Object | Core responsibility | Backend responsibility |
|---|---|---|
| runtime | public opaque handle, options validation, storage/allocator policy, lifecycle ordering | backend runtime state, backend identity, backend capability record |
| node | public opaque handle, runtime ownership, option validation | backend node state only when the backend needs it |
| endpoint | public endpoint identity and node ownership | backend endpoint resources and matching support |
| executor | public executor handle and option validation | poll or scheduling integration hooks when supported |
| diagnostics | result category mapping and diagnostic record shape | backend-specific detail codes and degraded-state evidence |
| capability | public root-summary structs and negotiation result shape | backend capability source data and descriptor-family contents |

Backend-private pointers must remain hidden behind core-owned opaque handles. Public headers must
not expose native socket types, RTOS task handles, DDS handles, `rcl`, `rmw`, `rclcpp`, C++ types,
or simulator implementation objects.

## Function Table Shape

M1 materializes the backend SPI in `src/grcl-c/include/grcl/c/backend.h`. The header is a
backend-implementer contract, not an application-facing runtime API. It may be installed with
`grcl-c`, but app SDKs must continue to use `runtime.h` and must not call backend operations
directly.

The G3 design baseline uses one backend descriptor, one backend operations table, one core-owned
runtime context, and one backend-owned runtime state pointer:

```c
typedef struct grcl_backend_runtime_context grcl_backend_runtime_context_t;
typedef struct grcl_backend_runtime_state grcl_backend_runtime_state_t;

typedef struct grcl_backend_runtime_context {
  size_t struct_size;
  uint32_t abi_version;
  const grcl_runtime_options_t * runtime_options;
  const grcl_storage_t * storage;
  const grcl_allocator_config_t * allocator;
} grcl_backend_runtime_context_t;

typedef struct grcl_backend_descriptor {
  size_t struct_size;
  uint32_t abi_version;
  const char * backend_name;
  uint32_t backend_family_id;
  uint32_t backend_version_major;
  uint32_t backend_version_minor;
  uint32_t backend_version_patch;
  const grcl_backend_ops_t * ops;
} grcl_backend_descriptor_t;

typedef struct grcl_backend_ops {
  size_t struct_size;
  uint32_t abi_version;
  grcl_result_t (*create_runtime)(
    const grcl_backend_runtime_context_t * context,
    grcl_backend_runtime_state_t ** out_backend_state);
  grcl_result_t (*start_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*stop_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*destroy_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*get_capabilities)(
    const grcl_backend_runtime_state_t * backend_state,
    grcl_runtime_capability_record_t * out_record);
  grcl_result_t (*negotiate_capabilities)(
    const grcl_backend_runtime_state_t * backend_state,
    const grcl_runtime_capability_request_t * request,
    grcl_capability_negotiation_result_t * out_result);
  grcl_result_t (*get_diagnostics)(
    const grcl_backend_runtime_state_t * backend_state,
    grcl_diagnostic_record_t * out_record,
    size_t record_capacity,
    size_t * out_record_count);
} grcl_backend_ops_t;
```

Rules:

- `grcl_backend_runtime_context_t` is core-owned and valid only for the duration of
  `create_runtime`.
- `grcl_backend_runtime_state_t` is backend-owned and opaque to the core except for lifetime.
- The core stores the backend state pointer inside the core-owned runtime handle.
- `destroy_runtime` receives the backend state pointer exactly once after successful creation, even
  if `start_runtime` fails.
- `get_diagnostics` uses a caller-provided output buffer. A zero capacity query may be used to ask
  for the required record count.
- Node, endpoint, executor, service/client, parameter, transport, graph-delta, and channel
  operations were not required in M1. M3 may extend the operation table through trailing fields
  guarded by `struct_size`.

## M3 SPI v0.2 Append-Only Extension

M3 extends `grcl_backend_ops_t` by appending operation pointers after the existing M1 fields. The
core must check `ops->struct_size` before calling any appended pointer so older lifecycle-only
backends remain detectable and can return unsupported results instead of being called through
missing hooks.

The v0.2 extension covers only local in-process behavior:

- node create and destroy notification.
- publisher, subscription, service, and client create and destroy notification.
- publish bytes and subscription take bytes.
- client request send, service request take, service response send, and client response take.
- executor spin once.
- runtime-local parameter set, get, and list.

The exact append order for M3-C is:

```c
typedef struct grcl_backend_node_state grcl_backend_node_state_t;
typedef struct grcl_backend_publisher_state grcl_backend_publisher_state_t;
typedef struct grcl_backend_subscription_state grcl_backend_subscription_state_t;
typedef struct grcl_backend_service_state grcl_backend_service_state_t;
typedef struct grcl_backend_client_state grcl_backend_client_state_t;
typedef struct grcl_backend_executor_state grcl_backend_executor_state_t;

struct grcl_backend_ops {
  /* M1 fields stay first and unchanged through get_diagnostics. */

  grcl_result_t (*create_node)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_node_t * node,
    const grcl_node_options_t * options,
    grcl_backend_node_state_t ** out_backend_node);
  grcl_result_t (*destroy_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*create_publisher)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_publisher_t * publisher,
    const grcl_publisher_options_t * options,
    grcl_backend_publisher_state_t ** out_backend_publisher);
  grcl_result_t (*destroy_publisher)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_publisher_state_t * backend_publisher);
  grcl_result_t (*create_subscription)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_subscription_t * subscription,
    const grcl_subscription_options_t * options,
    grcl_backend_subscription_state_t ** out_backend_subscription);
  grcl_result_t (*destroy_subscription)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_subscription_state_t * backend_subscription);
  grcl_result_t (*publish_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_publisher_state_t * backend_publisher,
    const void * payload,
    size_t payload_size);
  grcl_result_t (*subscription_take_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_subscription_state_t * backend_subscription,
    void * out_payload,
    size_t payload_capacity,
    size_t * out_payload_size);
  grcl_result_t (*create_service)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_service_t * service,
    const grcl_service_options_t * options,
    grcl_backend_service_state_t ** out_backend_service);
  grcl_result_t (*destroy_service)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service);
  grcl_result_t (*create_client)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_client_t * client,
    const grcl_client_options_t * options,
    grcl_backend_client_state_t ** out_backend_client);
  grcl_result_t (*destroy_client)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client);
  grcl_result_t (*client_send_request_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client,
    const void * request_payload,
    size_t request_payload_size,
    grcl_request_id_t * out_request_id);
  grcl_result_t (*service_take_request_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service,
    void * out_request_payload,
    size_t request_payload_capacity,
    size_t * out_request_payload_size,
    grcl_request_id_t * out_request_id);
  grcl_result_t (*service_send_response_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service,
    grcl_request_id_t request_id,
    const void * response_payload,
    size_t response_payload_size);
  grcl_result_t (*client_take_response_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client,
    grcl_request_id_t request_id,
    void * out_response_payload,
    size_t response_payload_capacity,
    size_t * out_response_payload_size);
  grcl_result_t (*create_executor)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_executor_t * executor,
    const grcl_executor_options_t * options,
    grcl_backend_executor_state_t ** out_backend_executor);
  grcl_result_t (*destroy_executor)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor);
  grcl_result_t (*executor_add_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*executor_remove_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*executor_spin_once)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    uint64_t timeout_ns);
  grcl_result_t (*runtime_param_set)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_param_record_t * param);
  grcl_result_t (*runtime_param_get)(
    grcl_backend_runtime_state_t * backend_state,
    const char * name,
    grcl_param_record_t * out_param,
    void * value_buffer,
    size_t value_buffer_capacity,
    size_t * out_value_size);
  grcl_result_t (*runtime_param_list)(
    grcl_backend_runtime_state_t * backend_state,
    char * out_names,
    size_t names_capacity,
    size_t * out_names_size,
    size_t * out_param_count);
};
```

Rules:

- The core owns public handles and validates public options.
- The backend owns backend-private queues, topic maps, service maps, request correlation, and
  parameter storage.
- Backend callbacks receive core-owned opaque object handles only as identity tokens. They must not
  dereference private core structs.
- Backend-private state typedefs belong in `backend.h` as incomplete structs. The native backend
  provides the definitions privately.
- Before calling any M3 field, the core must verify both field presence and non-null function
  pointer:
  `ops->struct_size >= offsetof(grcl_backend_ops_t, field) + sizeof(ops->field)` and
  `ops->field != NULL`.
- Missing or null M3 fields must return `GRCL_ERROR_UNSUPPORTED_CAPABILITY` from the public
  `grcl-c` API. The core must not call through a missing backend hook.
- The in-process native backend may copy bytes into backend-owned memory. It must not expose
  transport handles, sockets, pthread handles, ROS2/DDS handles, simulator objects, or C++ types.
- M3 does not add plugin loading, dynamic backend selection, network transport, shared memory,
  zero-copy handoff, background worker threads, or blocking waits.

The v0.2 append order is grouped by dependency: node state, pub/sub endpoints, service/client
endpoints, executor state, then runtime-local params. M3-C may add helper macros for field-presence
checks, but those helpers must be private to the core unless an explicit public ABI need appears.

## Required SPI Areas

| Area | Required responsibilities for G3 design | Deferred responsibilities |
|---|---|---|
| registration | backend descriptor, family id, ABI version, operation table sizing | dynamic plugin loading, registry, signing |
| runtime lifecycle | create, start, stop, destroy hooks mapped to public lifecycle | transport sessions |
| capability | fill `grcl_runtime_capability_record_t`, negotiate `grcl_runtime_capability_request_t` | full descriptor iterators, wire-format capability exchange |
| diagnostics | return backend diagnostic records without expanding `grcl_result_t` | management-plane snapshot schema |
| graph | report graph projection support and backend-owned graph deltas when available | full distributed graph cache |
| memory/storage | consume core-provided allocator/storage policy and report bounded-capacity failures | backend-specific zero-copy handoff |
| scheduling | expose whether polling or executor hooks are supported | final executor scheduling semantics |
| local messaging | local topic queues, request/reply queues, executor pull dispatch, runtime-local params | network transport, ROS2 projection, distributed graph, distributed params |
| local params capability | report runtime-local param limits through M3 append-only capability fields | distributed param descriptors, constraints, callbacks |

## Runtime Lifecycle Contract

Backend lifecycle hooks must preserve the public lifecycle order:

```text
registered -> runtime created -> started -> stopped -> destroyed
```

Rules:

- `create_runtime` validates backend-required storage, allocator, profile, and capability inputs.
- `start` may fail with compatibility or resource errors when required backend capabilities are
  unavailable.
- `stop` must be idempotent enough for cleanup after partial start failure.
- `destroy` must release backend-private state in the order requested by the core lifecycle owner.
- Backends must not create public nodes, endpoints, or executors outside core ownership.
- Backend errors map to `grcl_result_t` categories plus diagnostics. They must not add backend-only
  public result enums.

## Capability And Negotiation Contract

Every backend must be able to provide the fixed root-summary data needed by
`grcl_runtime_capability_record_t`:

- runtime identity, profile id, runtime class id, implementation id, and protocol/schema versions.
- supported, preferred, and default graph projection modes.
- summary flags for storage, executor, diagnostics, security-family presence, and graph behavior.
- bounded payload, entity, node, endpoint, session, channel, buffer, and storage capacities.
- descriptor-family counts or capacities for transport, QoS, encoding, diagnostics, and security
  families.

Negotiation rules:

- Required request fields produce `accepted` or `rejected_incompatible`.
- Optional request fields may produce `degraded_accepted`, but silent downgrade is forbidden.
- Capability, availability, and health remain separate. A backend must not report transient
  resource pressure as static capability.
- G3 must not require a new schema or opaque serialized blob for the first M1 native/null backend.

## Graph And ROS2 Projection Rule

The backend SPI owns graph projection hooks, but public GRCL APIs own the observed graph semantics.

The ROS2 backend must adapt ROS2 behavior without exposing `rcl`, `rmw`, or `rclcpp` through public
GRCL APIs. It must also not assume that ROS2 directly provides GRCL's runtime participant graph.
The adapter is responsible for projecting the GRCL runtime view from ROS2 node graph, backend
metadata, and GRCL-owned state.

G3 should design projection hooks narrowly enough that the M1 null/native backend can report static
or empty graph projection support without implementing ROS2 or transport behavior.

## MCU Backend Rule

The MCU backend must implement only the declared profile. Unsupported capabilities must fail fast
through capability negotiation or local object creation, not by partial runtime behavior.

For MCU/RTOS compatibility, G3 SPI structs and operation tables must preserve:

- explicit `struct_size` and `abi_version` fields.
- caller-provided allocator and storage policy.
- no mandatory heap allocation after init for profiles that declare `no_heap_after_init`.
- bounded descriptor counts and caller-provided output buffers for variable families.

## M1 Minimum Native/Null Backend Target

M1 implemented the smallest backend that proves the SPI can support the existing `grcl-c` contract:

| Scenario | Required result |
|---|---|
| create runtime with valid options | returns `GRCL_OK` and produces a core-owned runtime handle |
| init runtime with bounded storage | succeeds when required storage regions are present |
| start then stop runtime | preserves lifecycle order and returns `GRCL_OK` |
| query capabilities | returns a deterministic capability root summary |
| negotiate compatible request | returns `GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED` |
| negotiate optional downgrade | returns `GRCL_CAPABILITY_NEGOTIATION_STATUS_DEGRADED_ACCEPTED` with explicit downgrade evidence |
| negotiate incompatible request | returns `GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE` |
| invalid argument | returns `GRCL_ERROR_INVALID_ARGUMENT` |
| bad lifecycle state | returns `GRCL_ERROR_BAD_STATE` |
| missing bounded storage | returns `GRCL_ERROR_CAPACITY_EXCEEDED` with diagnostics |

M1 must not implement publish/subscribe, service/client, transport sessions, executor scheduling,
ROS2 projection, management snapshots, auth, remote management, event streams, SDK wrappers, build
systems, Dockerfiles, or CI workflows unless a later user-approved goal explicitly expands scope.

The completed M1 closeout verifies this target through `src/grcl-c/tests/run_m1_tests.sh`, including
C11/C++17 header smoke coverage and runnable lifecycle, capability, negotiation, diagnostics, and
bounded-storage negative tests.

## M3 Native In-Process Backend Target

M3 introduces a complete in-process native backend sufficient for core examples. "Complete" in M3
means complete for local C examples, not complete as a production transport backend.

M3-D uses a narrower bridge before the full native in-process backend exists: the existing
`null/native-test` backend may implement no-op M3 object-lifecycle hooks so the core can prove
runtime-owned node, endpoint, service/client, and executor lifetime rules through TDD. Those no-op
hooks are not routing hooks. They must not store topic queues, route requests, implement params,
claim local messaging capability, or hide missing M3 behavior. Pub/sub delivery, service/client
request/reply routing, executor dispatch, and runtime-local params remain owned by the later
in-process native backend batches.

M3-E selects the in-process native backend through existing runtime options, not through a new
public backend selector. A runtime with `grcl_runtime_options_t.profile_name` equal to
`"native-inprocess"` uses the M3 in-process backend. A runtime with no profile name, or any other
profile name, continues to use `null/native-test` until a future backend-selection contract is
designed.

| Scenario | Required result |
|---|---|
| create two nodes in one runtime | nodes are core-owned and backend observes private node state |
| create publisher and subscription with matching topic and type id | endpoints are accepted |
| publish bytes, spin once, take bytes | copied payload is delivered deterministically |
| publish with mismatched type id | no compatible subscription receives data |
| send service request, spin, service take, service respond, spin, client take | request id correlation is preserved |
| set/get/list local runtime params | backend-owned param table copies values and reports capacity errors |
| run C examples | examples compile and run without build-system rollout |

Capability reporting for M3 runtime-local params uses `GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS`
and these append-only fields in `grcl_runtime_capability_record_t`: `max_parameters`,
`parameter_name_buffer_bytes`, and `parameter_value_buffer_bytes`. The native backend must set those
fields only for runtime-local param support. It must not imply node-scoped params, distributed
params, callbacks, constraints, or watchers.

M3 must not implement sockets, shared memory, ROS2, simulator runtime, MCU runtime, gateway,
management snapshots, auth, remote management, event streams, package/build policy, or external
`grcl` migration.

## M5 Native-Backend Product Completion Rule

M5 is not a backend-family expansion goal. Its SPI responsibility is narrower: determine whether
the current native in-process backend plus the existing public `grcl-c` surface is already
sufficient to support full local-core use from both C and `grcl-cpp`.

Rules:

- M5 should prefer zero SPI expansion. If the current operation table and native backend behavior
  already support the approved C surface, `grcl-cpp` must wrap that surface rather than reopening
  SPI design.
- Any M5 SPI change must be justified by a failing local-core C or C++ test/example that cannot be
  fixed in the core or wrapper layers without changing backend contract behavior.
- Any M5 SPI change must preserve append-only `struct_size` discipline and backend containment.
- M5 must not introduce transport hooks, thread hooks, wait hooks, discovery streams, graph cache
  hooks, or distributed param hooks.

### M5 Native Backend Acceptance Matrix

The native backend is sufficient for M5 only if all of the following remain true:

| Area | Required native-backend behavior | Required evidence |
|---|---|---|
| lifecycle/materialization | core-owned public objects remain valid and backend-private state stays private | `src/grcl-c/tests/run_m1_tests.sh`, `src/grcl-c/tests/core_lifecycle_contract_test.c` |
| local pub/sub | publish copies into backend-owned storage, `spin_once` drives deterministic delivery, and `take` reports no-data and capacity boundaries correctly | `src/grcl-c/tests/pub_sub_test.c`, `src/grcl-c/tests/core_messaging_contract_test.c`, `examples/c/pub_sub_example.c` |
| local service/client | request ids remain correlated through spin-driven local request/reply | `src/grcl-c/tests/service_client_test.c`, `src/grcl-c/tests/core_messaging_contract_test.c`, `examples/c/service_client_example.c` |
| executor integration | backend participates only through bounded, non-blocking `spin_once` behavior | `src/grcl-c/tests/core_messaging_contract_test.c`, all M3 C examples |
| runtime-local params | backend-owned param table copies values and reports capacity boundaries | `src/grcl-c/tests/params_test.c`, `src/grcl-c/tests/core_params_capability_contract_test.c`, `examples/c/params_example.c` |
| capability/diagnostics | native backend reports only implemented local-core capability and diagnostics facts | `src/grcl-c/tests/backend_capability_test.c`, `src/grcl-c/tests/diagnostics_negative_state_test.c`, `src/grcl-c/tests/core_params_capability_contract_test.c` |

If M5 C++ parity work reveals a missing behavior here, the first question is whether the gap is a
wrapper gap or a core gap. Reopen SPI only when the evidence shows the native backend contract
itself is insufficient.

## Baseline Decisions

- Backend implementations are internal to the `grcl-c` core contract boundary.
- The core owns public handles; the backend owns backend-private state.
- Backend registration uses an ABI-versioned descriptor and operation table.
- G3 must define lifecycle and capability hooks before native/null backend implementation starts.
- Capability hooks must use the existing G2 capability structs and preserve capability,
  availability, and health separation.
- ROS2 backend projection is owned by the adapter and cannot rely on ROS2/rmw exposing a GRCL
  runtime participant graph.
- MCU backends implement declared profiles rather than full GRCL by default.

## Remaining Contract Gates

- M1 created `backend.h`, local tests, `src/grcl-c/src/runtime.c`, and the private
  `src/grcl-runtime-native/src/null_backend.c` null/native-test backend.
- Future post-M1 backend work must preserve the operation table shape above unless implementation
  evidence exposes a blocking lifecycle or capability issue.
- Transport ownership and zero-copy handoff rules remain deferred until native and MCU transport
  design goals.
- Publish/subscribe, service/client, executor scheduling, and graph delta semantics remain outside
  the completed M1 minimum target and enter scope only through the approved M3 in-process native
  backend goal.
- M5 uses the completed M3/M4 native backend as the current local-core product baseline and should
  avoid widening SPI scope unless failing parity evidence proves a real backend-contract gap.
