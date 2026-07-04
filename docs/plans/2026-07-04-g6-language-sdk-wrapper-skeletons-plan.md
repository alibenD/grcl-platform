# G6 Language SDK Wrapper Skeletons Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task after explicit
> implementation approval. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish M2 Cross-Language SDK Boundary Baseline by planning minimal `grcl-cpp` and
`grcl-py` wrapper skeletons over `grcl-c` without allowing either SDK to define independent runtime
semantics.

**Architecture:** G6 treats `grcl-c` as the only semantic source. `grcl-cpp` may provide C++ RAII
and result wrappers over `grcl-c` handles. `grcl-py` may provide Pythonic ownership and error
mapping only through a private native binding boundary that mirrors `grcl-c`; it must not bind the
external prototype repository or make `grcl-cpp` the semantic source. G6 implementation cannot start
until the user approves a new post-G5 implementation window and resolves the Python binding
substrate gate recorded below.

**Tech Stack:** Documentation-first planning; future implementation may use C++17 header/source
wrappers, Python 3 standard-library or native-shim boundary code, existing C headers under
`src/grcl-c/include/grcl/c/`, and local artifact-root-aware verification scripts. No CI, Docker,
repo-wide build system, package-manager rollout, ROS2 adapter, pub/sub, graph runtime, management
plane, or external `grcl` migration is part of this plan.

---

## Scope

G6 planning includes:

- SDK boundary decisions for `grcl-cpp` and `grcl-py`.
- dependency-ordered implementation task breakdown for a future approved G6 implementation window.
- allowed file sets and non-goals for future implementation subagents.
- local verification strategy that composes with the completed G5 conformance harness.
- durable status and navigation updates.

G6 planning does not include:

- expanding existing `src/grcl-cpp/` or `src/grcl-py/` module skeletons with SDK source code.
- implementing SDK wrapper code.
- introducing a CMake, Python package, wheel, uv, pip, pybind11, cffi, setuptools, scikit-build, or
  repo-wide build-system policy.
- modifying `grcl-c` ABI headers, runtime implementation, backend implementation, schemas, fixtures,
  M1 harness internals, or the G5 runner.
- implementing pub/sub, service/client, executor scheduling, graph behavior, transport, ROS2
  adapter behavior, MCU runtime behavior, simulator runtime behavior, management-plane behavior,
  auth, remote management, event streams, CI, Docker, or external `github.com/alibenD/grcl`
  migration.

## Canonical Design Home

The canonical design home for G6 is
[Language SDK Strategy](../architecture/language-sdk-strategy.md). SDK governance risk classes
remain in [SDK Platform Governance](../architecture/sdk-platform-governance.md). This plan must not
become the long-lived SDK design explanation.

## Planning Baseline

G6 starts from these completed artifacts:

- `grcl-c` public contract and M1 runnable lifecycle/capability/diagnostics path.
- backend SPI baseline materialized enough for M1 null/native-test behavior.
- G5 local conformance runner at `scripts/run-conformance.sh`.
- language SDK ADRs that require SDKs to wrap `grcl-c` rather than define independent semantics.

G6 does not require a new C ABI decision if wrappers only cover the M1-proven surface:

- runtime create or init-with-storage.
- runtime start, stop, and destroy.
- capability query and negotiation.
- diagnostics retrieval.
- result-code mapping.
- handle ownership and lifetime.

## User Decision Gate Before Implementation

G6 planning completed before this decision, and the user then selected Option A as the future G6
implementation substrate. G6 implementation still must not start until the user approves a new G6
implementation goal window.

| Option | Meaning | Trade-off |
|---|---|---|
| A: `ctypes`/private dynamic-library shim boundary | `grcl-py` owns a Python wrapper over a private native boundary shaped like `grcl-c`; implementation may initially validate import/static semantics and defer real shared-library packaging | Smallest dependency footprint, but real runtime calls require a later shared-library build decision |
| B: Python API skeleton only, no native calls | `grcl-py` documents and tests wrapper ownership semantics without calling `grcl-c` yet | Avoids build decisions, but weaker evidence for "wraps `grcl-c`" |
| C: native extension stack now | Introduce pybind11, cffi, CPython extension, or packaging/build-system policy during G6 | Stronger eventual runtime path, but too broad unless the user explicitly approves build/package scope |

Recommended planning baseline: Option A for future G6 implementation, with a stop condition if the
implementation tries to solve package distribution or shared-library production inside G6.

Selected decision: Option A. `grcl-py` should use a `ctypes` or private dynamic-library shim
boundary shaped like `grcl-c`, while package distribution, wheel production, generated shared
libraries, and native extension framework selection remain deferred.

## Allowed Files For Future G6 Implementation

Future G6 implementation task briefs may permit only the specific files they need, chosen from this
candidate set:

- `src/grcl-cpp/README.md`
- `src/grcl-cpp/include/grcl/cpp/*.hpp`
- `src/grcl-cpp/tests/*.cpp`
- `src/grcl-cpp/tests/run_g6_cpp_tests.sh`
- `src/grcl-py/README.md`
- `src/grcl-py/grcl_py/*.py`
- `src/grcl-py/tests/*.py`
- `src/grcl-py/tests/run_g6_py_tests.sh`
- `scripts/check-sdk-boundaries.py`
- `scripts/run-conformance.sh`
- `docs/architecture/language-sdk-strategy.md`
- `docs/architecture/sdk-platform-governance.md`
- `docs/architecture/conformance-and-ci-strategy.md`
- `docs/status/current-context.md`
- `docs/status/goal-execution-queue.md`
- `docs/status/middleware-goal-roadmap.md`
- `docs/README.md`
- `README.md`

Future G6 implementation must not edit:

- `src/grcl-c/include/grcl/c/*.h`
- `src/grcl-c/src/runtime.c`
- `src/grcl-runtime-native/src/null_backend.c`
- `src/grcl-c/tests/*`
- `schemas/*.yaml`
- `tests/conformance/**/*.yaml`
- CI, Docker, package-manager, or repo-wide build-system files.

If an SDK wrapper cannot be implemented without changing `grcl-c` ABI, stop and report the needed
ABI decision instead of changing the ABI inside G6.

## Future Task Queue

### G6-A: Planning And Durable State

**Files:**

- Create: `docs/plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md`
- Modify: `docs/architecture/language-sdk-strategy.md`
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `docs/architecture/system-overview.md`
- Modify: `docs/README.md`
- Modify: `README.md`

**Steps:**

- [ ] Record M2/G6 as the next planned milestone after G5.
- [ ] Update the SDK canonical design home with the G6 boundary baseline.
- [ ] Add the G6 plan to durable navigation.
- [ ] Record that G6 implementation is not authorized until the user approves a new implementation
  window and resolves the Python binding substrate gate.
- [ ] Validate documentation hygiene and status consistency.
- [ ] Commit the planning changes separately from future implementation batches.

**Exit criteria:**

- G6 plan exists and is linked from durable recovery state.
- Status documents agree that G5 is complete and G6 planning is complete or in review.
- No SDK source code, build-system files, CI, Docker, runtime, backend, schema, fixture, ROS2,
  simulator, MCU runtime, management-plane, or external migration files are changed.

### G6-B: User Gate For Python Binding Substrate

**Files:**

- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/current-context.md`

**Required decision:**

Choose one:

- Option A: `ctypes` or private dynamic-library shim boundary, with packaging deferred.
- Option B: Python API skeleton only, no native calls in G6.
- Option C: native extension stack now, which requires widening G6 to include build/package policy.

**Exit criteria:**

- The selected implementation profile is recorded before any SDK source files are created.
- If Option C is selected, a new build/package planning gate is created before implementation.

### G6-C: `grcl-cpp` Minimal RAII Boundary Skeleton

**Files:**

- Modify: `src/grcl-cpp/README.md`
- Create: `src/grcl-cpp/include/grcl/cpp/result.hpp`
- Create: `src/grcl-cpp/include/grcl/cpp/runtime.hpp`
- Create: `src/grcl-cpp/tests/runtime_wrapper_smoke.cpp`
- Create: `src/grcl-cpp/tests/run_g6_cpp_tests.sh`

**Required behavior:**

- `grcl::Result` or equivalent maps directly to `grcl_result_t`.
- `grcl::Runtime` owns exactly one `grcl_runtime_t *`.
- copy construction and copy assignment are deleted.
- move construction and move assignment transfer ownership without duplicating the handle.
- destructor calls `grcl_runtime_destroy` only when it owns a non-null handle.
- `start()` and `stop()` forward to `grcl_runtime_start` and `grcl_runtime_stop`.
- no exceptions are required in G6; an exception facade may remain deferred.

**Verification commands:**

```bash
src/grcl-cpp/tests/run_g6_cpp_tests.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g6-cpp src/grcl-cpp/tests/run_g6_cpp_tests.sh
```

**Expected result:** commands exit `0`, compile the wrapper smoke source as C++17, link against the
existing M1 `grcl-c` runtime/null backend sources without adding a repo-wide build system, and write
outputs only under the artifact root.

### G6-D: `grcl-py` Minimal Ownership Boundary Skeleton

**Files:**

- Modify: `src/grcl-py/README.md`
- Create: `src/grcl-py/grcl_py/__init__.py`
- Create: `src/grcl-py/grcl_py/result.py`
- Create: `src/grcl-py/grcl_py/runtime.py`
- Create: `src/grcl-py/grcl_py/_native.py`
- Create: `src/grcl-py/tests/test_runtime_boundary.py`
- Create: `src/grcl-py/tests/run_g6_py_tests.sh`

**Required behavior for Option A baseline:**

- `_native.py` defines the private boundary that mirrors the required `grcl-c` calls.
- `Runtime` owns one native runtime handle reference and prevents double close.
- context-manager entry/exit maps to lifecycle ownership without changing `grcl-c` semantics.
- result mapping is explicit and does not invent new success/error categories.
- tests use a local fake native boundary object for ownership semantics only; real shared-library
  loading is deferred unless a separate build/package decision is approved.

**Verification commands:**

```bash
src/grcl-py/tests/run_g6_py_tests.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g6-py src/grcl-py/tests/run_g6_py_tests.sh
```

**Expected result:** commands exit `0`, run Python standard-library tests, and do not require pip,
uv, wheels, pybind11, cffi, setuptools, scikit-build, or a generated shared library.

### G6-E: SDK Boundary Drift Checks

**Files:**

- Create: `scripts/check-sdk-boundaries.py`
- Modify: `scripts/run-conformance.sh`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`

**Required behavior:**

- Reject SDK source references to backend-private symbols or direct runtime implementation internals.
- Reject public SDK references to ROS2 backend symbols, `rcl`, `rmw`, `rclcpp`, socket APIs,
  pthread APIs, Docker, CI, and repo-wide build-system declarations.
- Verify `grcl-cpp` includes public `grcl/c/*.h` headers rather than copying ABI declarations.
- Verify `grcl-py` routes through its private native boundary module rather than importing
  `grcl-cpp` as the semantic source.
- Add the SDK boundary check to the local conformance runner only after G6-C and G6-D pass.

**Verification commands:**

```bash
python3 scripts/check-sdk-boundaries.py
scripts/run-conformance.sh
```

**Expected result:** both commands exit `0`; generated reports write under
`GRCL_PLATFORM_ARTIFACT_ROOT/g6/sdk-boundaries/` or the workspace artifact root default.

### G6-F: Closeout And Independent Audit

**Files:**

- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/status/middleware-goal-roadmap.md`
- Modify: `docs/architecture/language-sdk-strategy.md`
- Modify: `docs/architecture/conformance-and-ci-strategy.md`
- Modify: `docs/README.md`
- Modify: `README.md`

**Required behavior:**

- Rerun G5 conformance plus G6 SDK checks from workspace root and repository root where applicable.
- Verify no repo-root generated `build/`, `install`, `log`, `__pycache__`, virtualenv, package, or
  wheel output exists.
- Verify no SDK source claims publish/subscribe, executor, transport, ROS2, simulator, MCU runtime,
  management-plane, auth, remote management, event streams, CI, Docker, package manager, or
  external migration behavior.
- Update durable state only after independent audit acceptance.

**Verification commands:**

```bash
git -C src/grcl-platform diff --check
src/grcl-platform/scripts/run-conformance.sh
GRCL_PLATFORM_ARTIFACT_ROOT=/tmp/grcl-platform-g6-closeout src/grcl-platform/scripts/run-conformance.sh
find src/grcl-platform -maxdepth 3 \( -name build -o -name install -o -name log -o -name __pycache__ -o -name .venv -o -name dist -o -name '*.egg-info' \) -print
```

**Expected result:** conformance commands exit `0`; artifact scan exits `0` with no output.

## Stop Conditions

Stop and ask the user before implementation if:

- Python binding requires selecting a package manager, native extension framework, shared-library
  production path, or repo-wide build-system policy.
- SDK wrappers require changing `grcl-c` public ABI.
- `grcl-cpp` or `grcl-py` needs behavior beyond the M1-proven lifecycle, capability, negotiation,
  diagnostics, or ownership surface.
- Verification requires CI, Docker, CMake, colcon, setuptools, uv, pip, wheels, pybind11, cffi,
  scikit-build, or generated shared-library distribution.
- The implementation wants to import or migrate the external `github.com/alibenD/grcl` prototype.
- SDK code begins to implement pub/sub, service/client, executor scheduling, graph behavior,
  transport, ROS2 adapter behavior, MCU runtime behavior, simulator runtime behavior,
  management-plane behavior, auth, remote management, or event streams.

## Audit Checklist

Independent audit must verify:

- G6 implementation stayed within the selected Python binding profile.
- SDK wrappers forward lifecycle, ownership, result, capability, negotiation, and diagnostics
  semantics to `grcl-c`.
- `grcl-cpp` does not expose ROS2 or backend-private types.
- `grcl-py` does not use `grcl-cpp` as the semantic source.
- No package/build/CI/Docker policy was introduced without an explicit gate.
- G5 conformance still passes after SDK skeleton additions.
- Generated artifacts stay under the workspace artifact root or explicit `/tmp` override.

## Planning Closeout

This document completes G6 planning only. G6 implementation requires:

1. user approval for a new post-G5 implementation window.
2. use of the selected Option A Python binding substrate profile.
3. file-based task briefs under `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/`.
4. implementation subagents plus independent audit subagents before queue advancement.
