# M3 Core Middleware Contract And Native In-Process Backend Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task after the design
> review gates pass. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `grcl-c` run core middleware examples for publish/subscribe, service/client, and
local runtime parameters through an in-process native backend while preserving the `grcl-c`
contract, backend SPI containment, and SDK boundary rules.

**Architecture:** M3 extends the existing M1 runtime host and backend SPI from lifecycle/capability
only into local in-process messaging. The core continues to own public handles and lifecycle order;
the backend owns backend-private routing queues and parameter storage. M3 remains single-process,
deterministic, executor-driven, and bytes-oriented.

**Tech Stack:** C11 public ABI and tests, C++17 header smoke compatibility, shell test runners,
local artifact root, existing Python conformance scripts, implementation subagents plus independent
audit subagents.

---

## Scope

M3 includes:

- `grcl-c` node, publisher, subscription, service, client, executor, type-support, and local
  runtime parameter API shape.
- backend SPI v0.2 append-only operation table design for in-process native routing.
- an in-process native backend that supports topic queues, service request/reply queues, and a
  runtime-local parameter table.
- C tests and C examples for pub/sub, service/client, params, and combined core middleware flow.
- local conformance runner integration after M3 tests and examples pass.

M3 excludes:

- ROS2, DDS, sockets, shared memory, multi-process transport, simulator backend, MCU runtime,
  gateway, management plane, auth, remote management, event streams, CI, Docker, package manager,
  CMake, colcon, IDL/codegen, external `grcl` migration, and C++/Python example acceptance.
- background threads, blocking waits, future handles, distributed params, node-scoped params, and
  zero-copy transport.

## Design Baseline

Selected defaults:

- Backend target: in-process native backend.
- Pub/sub delivery: executor pull. `publish` enqueues bytes; `spin_once` progresses routing; users
  call `take` to retrieve data.
- Message type model: opaque bytes plus type identity fields. No IDL, codegen, serialization, or
  introspection in M3.
- Service model: spin-driven request/reply. Client sends request bytes, executor routes the request,
  service takes request and sends response bytes, executor routes the response, client takes it.
- Params: runtime-local set/get/list with bytes values and scalar/string type tags.
- Examples: C examples only.

## Task Queue

### M3-A: File Design And Durable State

Files:

- Create: `docs/plans/2026-07-04-m3-core-middleware-native-backend-plan.md`
- Modify: `README.md`
- Modify: `docs/README.md`
- Modify: `docs/architecture/system-overview.md`
- Modify: `docs/architecture/grcl-c-api-shape.md`
- Modify: `docs/architecture/backend-spi-contract.md`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`

Steps:

- [ ] Record M3 as the active user-approved goal after G6.
- [ ] Fix stale G6 status in `system-overview.md`.
- [ ] Record M3 API, SPI, backend, test, and example design in canonical design homes.
- [ ] Add queue items for design review and implementation batches.
- [ ] Validate documentation hygiene.

Exit criteria:

- M3 design is discoverable from `docs/README.md`, `README.md`, current context, roadmap, and the
  relevant canonical design homes.
- No runtime source, backend source, header source, examples, or tests are changed in M3-A.

### M3-B: Independent Design Reviews

Files:

- `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-b-architecture-review/`
- `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-b-abi-review/`
- `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-b-test-plan-review/`

Steps:

- [ ] Dispatch architecture review for runtime/backend/SDK separation and forbidden scope.
- [ ] Dispatch ABI review for C naming, struct sizing, opaque handles, ownership, and append-only
  SPI extension.
- [ ] Dispatch test-plan review for TDD coverage, negative cases, capacity behavior, and examples.
- [ ] Fix design documents if any review rejects the plan.

Exit criteria:

- Architecture, ABI, and test-plan reviews return `accepted` or `accepted_with_notes`.
- Rejected reviews create narrow design-fix tasks before implementation.

### M3-C: API And SPI Header Contract

Files:

- Modify: `src/grcl-c/include/grcl/c/result.h`
- Modify: `src/grcl-c/include/grcl/c/types.h`
- Modify: `src/grcl-c/include/grcl/c/runtime.h`
- Modify: `src/grcl-c/include/grcl/c/backend.h`
- Modify: `src/grcl-c/include/grcl/c/capability.h`
- Modify: `src/grcl-c/tests/compile_headers_smoke.c`
- Modify: `src/grcl-c/tests/compile_headers_smoke.cpp`

Steps:

- [ ] Write failing C and C++ header smoke checks for the M3 types and functions.
- [ ] Add append-only result values required by M3 negative-path tests:
  `GRCL_ERROR_NOT_FOUND`, `GRCL_ERROR_NO_DATA`, `GRCL_ERROR_TYPE_MISMATCH`, and
  `GRCL_ERROR_PAYLOAD_TOO_LARGE`.
- [ ] Extend public type support, message byte APIs, service/client APIs, executor APIs, and local
  runtime parameter declarations.
- [ ] Define `grcl_type_support_t` as a public descriptor/value struct in `types.h`, not as an
  opaque handle.
- [ ] Append backend SPI v0.2 operation pointers after existing M1 fields.
- [ ] Add append-only capability representation for runtime-local params:
  `GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS`, `max_parameters`,
  `parameter_name_buffer_bytes`, and `parameter_value_buffer_bytes`.
- [ ] Verify header smoke fails before implementation and passes after header changes.

Exit criteria:

- Header smoke passes in C11 and C++17.
- No backend-private, ROS2, socket, pthread, or build-system types leak into public headers.

### M3-D: Core Object Ownership

Files:

- Modify: `src/grcl-c/src/runtime.c`
- Modify: `src/grcl-runtime-native/src/null_backend.c`
- Add or modify: `src/grcl-c/tests/object_ownership_test.c`
- Modify: `src/grcl-c/tests/run_m1_tests.sh`

Steps:

- [ ] Write failing tests for node ownership, node destroy, endpoint ownership, runtime destroy
  cleanup, invalid arguments, and bad lifecycle state.
- [ ] Write failing tests for core-owned node, endpoint, publisher, subscription, service, client,
  and executor table capacity exhaustion returning `GRCL_ERROR_CAPACITY_EXCEEDED`.
- [ ] Extend `null/native-test` with M3 object-lifecycle no-op backend hooks only. These hooks may
  acknowledge node, endpoint, and executor create/destroy so core ownership can be tested, but they
  must not route messages, allocate backend queues, implement params, or claim native in-process
  routing capability.
- [ ] Implement core-owned node, endpoint, publisher, subscription, service, client, and executor
  tables with deterministic destroy behavior.
- [ ] Keep backend-private state behind backend callbacks.

Exit criteria:

- Core ownership tests pass.
- Existing M1 lifecycle, capability, and diagnostics tests remain green.
- `null/native-test` remains a lifecycle/object-lifecycle test backend only; pub/sub delivery,
  service/client routing, executor dispatch, and params stay unsupported until later M3 batches.

### M3-E: Pub/Sub Routing

Files:

- Modify: `src/grcl-c/src/runtime.c`
- Modify: `src/grcl-runtime-native/src/null_backend.c` only if it remains a lifecycle-only backend
  with explicit unsupported results.
- Create: `src/grcl-runtime-native/src/inprocess_backend.c`
- Create: `src/grcl-c/tests/pub_sub_test.c`
- Modify: `src/grcl-c/tests/run_m1_tests.sh`

Steps:

- [ ] Write failing tests for publisher/subscription create, topic/type matching, publish byte copy,
  executor spin, take success, type mismatch, too-small receive buffer, and empty queue behavior.
- [ ] Write failing tests for publisher/topic queue saturation returning
  `GRCL_ERROR_CAPACITY_EXCEEDED` and payloads larger than `max_payload_bytes` returning
  `GRCL_ERROR_PAYLOAD_TOO_LARGE`.
- [ ] Implement in-process topic queues in backend-private state.
- [ ] Route publish through backend storage; route delivery only during `spin_once`.

Exit criteria:

- Pub/sub tests pass.
- Caller buffers may be reused after publish.
- No background threads or transport APIs are introduced.

### M3-F: Service/Client Routing

Files:

- Modify: `src/grcl-c/src/runtime.c`
- Modify: `src/grcl-runtime-native/src/inprocess_backend.c`
- Create: `src/grcl-c/tests/service_client_test.c`
- Modify: `src/grcl-c/tests/run_m1_tests.sh`

Steps:

- [ ] Write failing tests for service/client create, request send, service take request, response
  send, client take response, request id correlation, type mismatch, empty queue, and too-small
  buffers.
- [ ] Write failing tests for service request queue saturation, client response queue saturation,
  unknown request id, missing service, and payloads larger than `max_payload_bytes`.
- [ ] Implement request/reply queues driven by `spin_once`.
- [ ] Return explicit result codes for missing service, incompatible type, and empty response.

Exit criteria:

- Service/client tests pass without blocking waits, futures, background threads, or networking.

### M3-G: Local Runtime Params

Files:

- Modify: `src/grcl-c/include/grcl/c/runtime.h`
- Modify: `src/grcl-c/src/runtime.c`
- Modify: `src/grcl-runtime-native/src/inprocess_backend.c`
- Create: `src/grcl-c/tests/params_test.c`
- Modify: `src/grcl-c/tests/run_m1_tests.sh`

Steps:

- [ ] Write failing tests for set, get, overwrite, list, missing key, too-small output buffer, and
  invalid name.
- [ ] Write failing tests for runtime-local parameter table saturation, name-buffer saturation, and
  value-storage saturation returning `GRCL_ERROR_CAPACITY_EXCEEDED`.
- [ ] Implement runtime-local parameter table in backend-private state.
- [ ] Support bytes values with explicit type tags for bool, signed integer, unsigned integer,
  float64, string, and bytes.

Exit criteria:

- Param tests pass.
- No distributed params, node-scoped params, callbacks, or watchers are introduced.

### M3-H: Native Backend Capability Update

Files:

- Modify: `src/grcl-runtime-native/src/inprocess_backend.c`
- Modify: `src/grcl-c/tests/backend_capability_test.c`

Steps:

- [ ] Write failing tests that the in-process native backend reports bounded local pub/sub,
  service/client, executor pull, and runtime-local params support through capability summary fields
  and capacity fields.
- [ ] Verify runtime-local params are represented only by
  `GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS`, `max_parameters`,
  `parameter_name_buffer_bytes`, and `parameter_value_buffer_bytes`.
- [ ] Implement capability values without claiming transport, ROS2, graph-cache, distributed
  params, security, or remote management support.

Exit criteria:

- Capability tests pass.
- Capability, availability, and health remain separate.

### M3-I: C Examples

Files:

- Create: `examples/c/pub_sub_example.c`
- Create: `examples/c/service_client_example.c`
- Create: `examples/c/params_example.c`
- Create: `examples/c/core_middleware_example.c`
- Create: `examples/c/run_m3_examples.sh`

Steps:

- [ ] Write example sources that exit non-zero on behavior failure.
- [ ] Compile examples directly with C11 into the artifact root.
- [ ] Run examples from repository root, workspace root, and with `GRCL_PLATFORM_ARTIFACT_ROOT`
  override.

Exit criteria:

- All examples compile and run without CMake, colcon, package managers, CI, or Docker.

### M3-J: Conformance Integration And Closeout

Files:

- Modify: `scripts/run-conformance.sh`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `README.md`
- Modify: `docs/README.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`

Steps:

- [ ] Add M3 core middleware examples/tests as a local conformance stage after all M3 tests pass.
- [ ] Run full conformance from workspace root, repository root, and `/tmp` artifact override.
- [ ] Run artifact-pollution scan.
- [ ] Run SDK boundary drift check.
- [ ] Dispatch independent closeout audit.

Exit criteria:

- Full runner passes all stages.
- Artifact scan has no repository-root generated output.
- GQ queue and current context mark M3 complete only after closeout audit acceptance.

## Stop Conditions

Stop and report before implementation if:

- the design requires network transport, sockets, shared memory, background threads, ROS2, DDS,
  simulator, MCU runtime, gateway, management plane, auth, remote management, event streams,
  package/build policy, CMake, colcon, IDL/codegen, C++/Python example acceptance, or external
  `grcl` migration.
- API/SPI design requires a non-append-only ABI break that cannot be expressed as a v0.1-to-v0.2
  planned contract update.
- tests cannot prove executor pull behavior without adding hidden background work.
- service/client semantics require blocking waits, futures, or cross-runtime transport.

## Review Checklist

Independent reviewers must verify:

- `grcl-c` remains the only public semantic source.
- SDKs do not define backend or messaging semantics during M3.
- public headers do not leak backend-private, ROS2, socket, pthread, simulator, or C++ types.
- backend SPI keeps public objects core-owned and backend state backend-owned.
- all M3 behavior is test-first with observed failing tests before production changes.
- examples validate behavior but do not replace unit/contract tests.
