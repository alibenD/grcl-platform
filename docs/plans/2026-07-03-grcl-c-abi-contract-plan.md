# GRCL-C ABI Contract v0.1 Plan

## Purpose

This plan turns G2 from the middleware roadmap into an executable design and implementation
sequence for the `grcl-c` public ABI. It starts from the G1 contract artifacts and stops before
runtime behavior, backend SPI implementation, SDK implementation, build-system work, Docker work,
or external `grcl` migration.

G2 is not a backend goal. Backend SPI function-table shape remains G3. G2 may prepare public
types, lifecycle declarations, storage descriptors, result categories, capability query contracts,
and compile-only checks that future backend work must satisfy.

## Source Inputs

- [Middleware Goal Roadmap](../status/middleware-goal-roadmap.md)
- [GRCL-C Contract Boundary](../architecture/grcl-c-contract-boundary.md)
- [GRCL-C API Shape](../architecture/grcl-c-api-shape.md)
- [GRCL-C Capability ABI Representation](../architecture/grcl-c-capability-abi-representation.md)
- [Runtime Capability Schema](../architecture/runtime-capability-schema.md)
- [Runtime Capability Graph](../architecture/runtime-capability-graph.md)
- [MCU Profile Schema](../architecture/mcu-profile-schema.md)
- [Backend SPI Contract](../architecture/backend-spi-contract.md)
- [Conformance And CI Strategy](../architecture/conformance-and-ci-strategy.md)
- [ADR-0002 GRCL-C As Core Contract](../adr/ADR-0002-grcl-c-as-core-contract.md)
- [ADR-0005 GRCL-C Handle And Lifecycle Contract](../adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md)
- [ADR-0006 Runtime Capability Schema](../adr/ADR-0006-runtime-capability-schema.md)
- [ADR-0010 GRCL-C Capability ABI Representation](../adr/ADR-0010-grcl-c-capability-abi-representation.md)

G1 artifacts that G2 must preserve:

- `src/grcl-c/include/grcl/c/version.h`
- `src/grcl-c/include/grcl/c/result.h`
- `src/grcl-c/include/grcl/c/types.h`
- `src/grcl-c/include/grcl/c/runtime.h`
- `src/grcl-c/include/grcl/c/capability.h`
- `src/grcl-c/include/grcl/c/storage.h`
- `schemas/runtime-capability-record.schema.yaml`
- `schemas/capability-negotiation-result.schema.yaml`
- `schemas/mcu-profile.schema.yaml`
- `tests/conformance/runtime-capability/*.yaml`
- `tests/conformance/mcu-profiles/*.yaml`

## G2 Exit Criteria

G2 is complete when:

- public `grcl-c` headers compile in C and C++ compile-only smoke tests.
- public headers preserve opaque handles and expose no C++ or ROS2 implementation types.
- ABI rules for `struct_size`, `abi_version`, enum stability, ownership, and output buffers are
  explicit in documentation and headers.
- result categories, lifecycle declarations, node/endpoint declarations, storage descriptors,
  allocator surface, diagnostic-code surface, and capability query surface are coherent.
- capability query APIs preserve capability, availability, and health separation.
- no runtime behavior, backend implementation, SDK implementation, management plane implementation,
  auth, remote management, event stream, build scripts, Dockerfiles, package manifests, CI, or
  external `grcl` migration is introduced.

## Non-Goals

G2 must not:

- implement `grcl_runtime_create`, `grcl_runtime_start`, endpoint creation, publishing,
  subscription, service, client, executor, transport, or graph behavior.
- define backend SPI function tables or backend registration implementation.
- create a native, null, simulator, ROS2, MCU, gateway, C++, or Python implementation.
- create build scripts, package manifests, Dockerfiles, CI, or generated artifacts.
- implement management-plane snapshots, remote management, authentication, authorization, event
  streams, or SDK permission policy.
- migrate or mirror `github.com/alibenD/grcl`.

## Resolved Decision Gate Before G2 Implementation

G2 had one blocking ABI/schema representation decision:

```text
How should nested runtime capability schema data be represented across the public C ABI?
```

The G1 schema is nested and profile-dependent. A public C ABI cannot safely expose unconstrained
language-level containers, C++ types, YAML/JSON parser objects, ROS2 types, or unbounded dynamic
arrays. The user confirmed the recommended hybrid strategy for G2-A. This resolves GQ-030 and
allows G2-A closeout and later G2-B preparation after independent audit. It does not start G2-B or
G2-D header implementation in this task.

### Option A: Fixed Public Structs With Bounded Arrays

Represent capability data as nested C structs with fixed-size arrays and explicit counts.

Benefits:

- easy to inspect in C.
- simple compile-only conformance checks.
- friendly to MCU/RTOS profiles when capacities are bounded.

Costs:

- ABI becomes large and hard to evolve.
- every schema extension risks struct bloat.
- profile-specific fields can become awkward or wasteful.

Best fit:

- small stable core fields such as protocol version, profile, graph mode, entity limits, and basic
  memory limits.

### Option B: Descriptor Tables And Iterators

Represent variable parts as descriptor arrays or iterator APIs:

```c
grcl_result_t grcl_runtime_capability_get_transport(
  const grcl_runtime_capability_record_t * record,
  size_t index,
  grcl_transport_capability_t * out_transport);
```

Benefits:

- handles variable-length capability families without exposing dynamic containers.
- keeps the root ABI smaller.
- supports constrained profiles by requiring caller-provided buffers and count queries.

Costs:

- more API surface.
- more rules needed for buffer lifetime and stable iteration.
- still needs careful versioning for descriptor structs.

Best fit:

- transport lists, QoS variants, encoding sets, diagnostics capabilities, and security capability
  subsets.

### Option C: Opaque Serialized Capability Blob Plus Query Helpers

Expose an opaque capability payload and provide query helpers for selected fields.

Benefits:

- most flexible for schema evolution.
- easier to align with future wire protocol and management-plane payloads.

Costs:

- less transparent for MCU/RTOS and direct C users.
- requires serialization/versioning rules earlier.
- risks pushing too much behavior into opaque parsing before the core ABI is ready.

Best fit:

- future protocol and management-plane integration, not the first v0.1 public C ABI baseline.

### Selected Direction

Use the recommended hybrid:

- fixed root structs for stable identity, protocol, profile/class identifiers, bounded storage and
  entity limits, lifecycle/result summaries, and graph projection summaries.
- descriptor tables, iterator-style APIs, or caller-provided output buffer APIs for variable
  capability families including transports, QoS variants/subsets, encodings, diagnostics
  capabilities, and security capability subsets.
- no opaque serialized blob in G2 except as a future extension point.

This decision keeps G2 implementable without prematurely designing the full wire protocol or
management-plane snapshot schema. Capability, availability, and health remain separate concepts and
must not be collapsed into one ABI object.

## G2 Batch Breakdown

| Batch | Work | Requires Audit | Depends On |
|---|---|---|---|
| G2-A | Resolve capability C ABI representation and update ABI design notes | yes | user confirmed the recommended hybrid representation gate |
| G2-B | Harden version/result/types/storage/allocator/diagnostic headers | yes | G2-A |
| G2-C | Harden runtime lifecycle, node, endpoint, publisher, subscription, service, client declarations | yes | G2-B |
| G2-D | Add capability query and negotiation C ABI structs/functions consistent with G1 schemas | yes | G2-A, G2-B |
| G2-E | Add compile-only smoke sources or fixtures under the approved conformance location | yes | G2-B, G2-C, G2-D |
| G2-F | Documentation/status closeout for G2 and next G3 backend SPI planning gate | yes | G2-E |

## Allowed File Classes After Decision Gate

The exact allowed files must be listed in each task brief. Expected G2 implementation write areas
are limited to:

- `src/grcl-c/include/grcl/c/*.h`
- `src/grcl-c/tests/` for compile-only smoke sources or README/test-vector material
- `tests/conformance/` for C ABI conformance fixtures if needed
- `docs/status/` and README indexes for closeout only

Do not create build-system or runner files in G2. Compile-only validation may use temporary files
under `/tmp` or command-line snippets until a later conformance-harness goal creates durable
scripts.

## Verification Strategy

Every G2 implementation task must run the relevant subset of:

```bash
git -C src/grcl-platform status --short --untracked-files=all
```

```bash
git -C src/grcl-platform diff --check
```

```bash
cc -std=c11 -I src/grcl-platform/src/grcl-c/include -fsyntax-only /tmp/grcl_g2_header_smoke.c
```

```bash
c++ -std=c++17 -I src/grcl-platform/src/grcl-c/include -fsyntax-only /tmp/grcl_g2_header_smoke.cpp
```

```bash
rg -n 'std::|template|throw|class |namespace |rcl_|rmw_|rclcpp|<rcl|<rmw|<rclcpp' src/grcl-platform/src/grcl-c/include/grcl/c
```

The forbidden-token check must not use a bare `rcl` token because it falsely matches the required
`grcl_` prefix.

## Stop Conditions

Stop and report instead of implementing if:

- the selected capability representation cannot preserve schema evolution and MCU boundedness.
- lifecycle ownership requires backend behavior to be specified before G3.
- type support requires final serialization/codegen ABI beyond opaque identity and generated hook
  placeholders.
- compile-only validation requires creating build scripts or package manifests.
- any task would need runtime behavior, backend implementation, SDK implementation, management
  implementation, auth, remote management, event streams, Dockerfiles, CI, or external `grcl`
  migration.

## Immediate Next Step

G2-A is complete after independent audit acceptance.

The next step is to prepare the G2-B header-hardening brief with a narrowed allowed-write scope
over `src/grcl-c/include/grcl/c/*.h` and any minimal compile-only smoke artifacts that do not
introduce build scripts, package manifests, CI, or runtime behavior.
