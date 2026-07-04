# M1 First Runnable GRCL-C Core With Null/Native Backend Plan

## Purpose

M1 turns the accepted `grcl-c` contract and G3 backend SPI design into the first runnable middleware
core baseline. It must prove the path from public `grcl-c` lifecycle APIs through a backend SPI into
a deterministic null/native-test backend.

M1 is intentionally narrow. It validates lifecycle, capability query, negotiation, diagnostics, and
bounded-storage failure behavior. It does not implement transport, publish/subscribe,
service/client, executor scheduling, ROS2, MCU, SDK wrappers, management plane, auth, remote
management, event streams, Docker, CI, or external `grcl` migration.

## Source Inputs

- [Middleware Goal Roadmap](../status/middleware-goal-roadmap.md)
- [Goal Execution Queue](../status/goal-execution-queue.md)
- [GRCL-C API Shape](../architecture/grcl-c-api-shape.md)
- [Backend SPI Contract](../architecture/backend-spi-contract.md)
- [Runtime Capability Schema](../architecture/runtime-capability-schema.md)
- [GRCL-C Capability ABI Representation](../architecture/grcl-c-capability-abi-representation.md)
- [Conformance And CI Strategy](../architecture/conformance-and-ci-strategy.md)
- [Agentic Delivery Governance](../architecture/agentic-delivery-governance.md)
- [ADR-0002 GRCL-C As Core Contract](../adr/ADR-0002-grcl-c-as-core-contract.md)
- [ADR-0010 GRCL-C Capability ABI Representation](../adr/ADR-0010-grcl-c-capability-abi-representation.md)

## Exit Criteria

M1 is complete when:

- `backend.h` materializes the G3 backend SPI operation table and compiles in C and C++.
- `grcl-c` has a minimal runtime implementation for create, init-with-storage, start, stop,
  destroy, get capabilities, and negotiate capabilities.
- a deterministic null/native-test backend implements lifecycle, capability, negotiation, and
  diagnostic hooks.
- lifecycle and capability tests run locally and pass.
- negative tests cover invalid argument, bad lifecycle state, incompatible capability request, and
  missing bounded storage.
- build/test outputs write under the workspace `artifacts/` tree or temporary compiler outputs,
  not repository-root `build/`, `install/`, or `log/`.
- every implementation batch has an implementation report and independent audit report under
  `.local/agentic-runs/2026-07-04-m1-first-runnable-core/<task-id>/`.

## Non-Goals

M1 must not:

- implement publish/subscribe, service/client, executor scheduling, transport sessions, or graph
  delta behavior.
- implement ROS2, MCU, simulator, gateway, C++, Python, or Rust SDK behavior.
- implement management snapshots, auth, remote management, event streams, registry, signing, CI, or
  Docker.
- migrate or mirror `github.com/alibenD/grcl`.
- introduce a repository-wide build system or CI policy. A minimal local test runner is allowed
  only for M1 tests and must keep generated outputs outside the repository root.

## Batch Breakdown

| Batch | Work | Requires Audit | Depends On |
|---|---|---|---|
| M1-A | File this implementation plan and update durable queue state | no | G3 baseline |
| M1-B | Add backend SPI header and compile-only coverage | yes | M1-A |
| M1-C | Add minimal local M1 test harness | yes | M1-B |
| M1-D | Implement core runtime lifecycle skeleton | yes | M1-B, M1-C |
| M1-E | Implement null/native-test backend capability and negotiation behavior | yes | M1-D |
| M1-F | Add diagnostics and negative state tests | yes | M1-D, M1-E |
| M1-G | Run full M1 verification and close runnable-core milestone | yes | M1-F |

## Expected File Classes

Each task brief must list exact allowed files. Expected M1 write areas are limited to:

- `src/grcl-c/include/grcl/c/backend.h`
- `src/grcl-c/include/grcl/c/*.h` only when needed to expose M1 public declarations already
  accepted by the design baseline
- `src/grcl-c/src/` for minimal `grcl-c` implementation files
- `src/grcl-c/tests/` for compile-only and runnable M1 test artifacts
- `src/grcl-runtime-native/` only if used for the null/native-test backend implementation
- `docs/status/`, `docs/architecture/`, and `docs/README.md` for closeout updates only

## Minimum Runnable Behavior

The first runnable path is:

```text
grcl_runtime_create/init_with_storage
  -> core runtime object
  -> backend SPI create_runtime
  -> null/native-test backend state
  -> start/stop
  -> capability query and negotiation
  -> deterministic destroy
```

The test target must prove:

- create with valid options returns `GRCL_OK`.
- init with sufficient bounded storage returns `GRCL_OK`.
- missing bounded storage returns a resource category error.
- start before create is impossible through the public API; start with null runtime returns
  `GRCL_ERROR_INVALID_ARGUMENT`.
- repeated start or stop in the wrong state returns `GRCL_ERROR_BAD_STATE`.
- capability query returns deterministic root-summary fields.
- negotiation returns accepted, degraded accepted, and rejected incompatible cases.
- diagnostics can report at least the latest lifecycle or storage failure.

## Verification Strategy

Every implementation batch must run relevant subsets of:

```bash
git -C src/grcl-platform status --short --untracked-files=all
git -C src/grcl-platform diff --check
cc -std=c11 -I src/grcl-platform/src/grcl-c/include -fsyntax-only src/grcl-platform/src/grcl-c/tests/compile_headers_smoke.c
c++ -std=c++17 -I src/grcl-platform/src/grcl-c/include -fsyntax-only src/grcl-platform/src/grcl-c/tests/compile_headers_smoke.cpp
```

The full M1 closeout must also run the durable M1 local test command introduced by M1-C.

## Stop Conditions

Stop and report instead of continuing if:

- audit returns `rejected`.
- implementation requires a new public ABI, schema, or protocol decision beyond the accepted G2/G3
  baseline.
- build/test harness work expands into repository-wide build-system policy, Docker, or CI.
- backend SPI implementation requires changing the G3 ownership model.
- code begins pulling in transport, publish/subscribe, service/client, executor scheduling, ROS2,
  SDK, management, auth, remote management, event stream, or external `grcl` migration scope.
