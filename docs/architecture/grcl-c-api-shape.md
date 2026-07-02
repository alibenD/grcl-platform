# GRCL-C API Shape

## Purpose

This document defines the first public shape of the `grcl-c` contract before code is written. It
is intentionally an API design document, not a header implementation.

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
| `grcl_type_support_t` | type identity, encoding, and generated operations |
| `grcl_backend_t` | backend implementation handle |
| `grcl_transport_t` | transport implementation handle |
| `grcl_allocator_t` | allocator policy handle |
| `grcl_storage_t` | caller-provided controlled storage descriptor |

## Result Model

`grcl_result_t` must separate permanent incompatibility from transient runtime state.

| Category | Example Codes |
|---|---|
| success | `GRCL_OK` |
| caller error | `GRCL_ERROR_INVALID_ARGUMENT`, `GRCL_ERROR_BAD_STATE` |
| resource error | `GRCL_ERROR_OUT_OF_MEMORY`, `GRCL_ERROR_CAPACITY_EXCEEDED` |
| compatibility error | `GRCL_ERROR_UNSUPPORTED_CAPABILITY`, `GRCL_ERROR_INCOMPATIBLE_VERSION` |
| runtime error | `GRCL_ERROR_TIMEOUT`, `GRCL_ERROR_INTERRUPTED`, `GRCL_ERROR_BACKEND_FAILURE` |
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
```

Rules:

- Capability queries must not report current resource availability as static capability.
- Negotiation must return `accepted`, `degraded_accepted`, or `rejected_incompatible`.
- Silent downgrade is not allowed.

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

