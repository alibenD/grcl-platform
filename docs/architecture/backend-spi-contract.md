# Backend SPI Contract

## Purpose

This document is the canonical design home for the backend SPI. It defines the internal adapter
boundary that backend implementations must satisfy so the `grcl-c` core contract can host native,
ROS2, MCU/RTOS, gateway, and simulator runtimes without allowing any one backend to define public
GRCL semantics by accident.

G3 produces this SPI shape as a design baseline. It does not implement a backend and does not add
runtime behavior.

## Backend Families

| Backend | Role | First implementation goal |
|---|---|---|
| null/native-test | deterministic local backend used to validate lifecycle and capability contracts | completed in M1 minimum runnable-core scope |
| native | Linux/native transport and local runtime behavior | after a future approved post-M1 plan |
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
- Node, endpoint, executor, transport, graph-delta, and channel operations are not required in M1.
  Later goals may extend the operation table through trailing fields guarded by `struct_size`.

## Required SPI Areas

| Area | Required responsibilities for G3 design | Deferred responsibilities |
|---|---|---|
| registration | backend descriptor, family id, ABI version, operation table sizing | dynamic plugin loading, registry, signing |
| runtime lifecycle | create, start, stop, destroy hooks mapped to public lifecycle | publish/subscribe behavior, transport sessions |
| capability | fill `grcl_runtime_capability_record_t`, negotiate `grcl_runtime_capability_request_t` | full descriptor iterators, wire-format capability exchange |
| diagnostics | return backend diagnostic records without expanding `grcl_result_t` | management-plane snapshot schema |
| graph | report graph projection support and backend-owned graph deltas when available | full distributed graph cache |
| memory/storage | consume core-provided allocator/storage policy and report bounded-capacity failures | backend-specific zero-copy handoff |
| scheduling | expose whether polling or executor hooks are supported | final executor scheduling semantics |

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
  the completed M1 minimum target.
