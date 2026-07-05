# Conformance And CI Strategy

## Purpose

GRCL Platform needs conformance before implementation expands because the system spans C ABI,
language SDKs, native Linux runtimes, ROS2 adapters, MCU/RTOS profiles, gateways, simulators, and
future third-party SDKs. Tests must verify that these surfaces implement one architecture rather
than independent interpretations.

This document defines the conformance layers, CI stages, environment ownership, and preconditions
for future implementation plans.

## Non-Goals

- Do not implement test scripts in this document.
- Do not create CI workflow files yet.
- Do not claim ROS2 runtime verification from macOS.
- Do not define final protocol byte layouts.
- Do not treat the G5 local harness as CI, Docker, repo-wide build-system, SDK, pub/sub, graph,
  transport, ROS2, MCU runtime, simulator, or management-plane implementation.

## Conformance Layers

| Layer | Purpose | Initial Evidence |
|---|---|---|
| Documentation conformance | Keep architecture, ADRs, plans, and status files internally consistent | markdown link checks, unfinished-marker scan, diff hygiene |
| Schema conformance | Validate runtime capability records, negotiation results, and MCU profile descriptors | YAML schemas and static test vectors |
| C ABI conformance | Ensure `grcl-c` headers preserve opaque handles, result categories, ABI versioning, and storage rules | header checks and compile-only tests |
| Cross-language conformance | Ensure C++, Python, and future SDKs wrap `grcl-c` without independent semantics | behavior-equivalence scenarios |
| Runtime capability conformance | Ensure capability, availability, health, graph projection, and negotiation results remain separated | accepted/degraded/rejected vectors |
| MCU profile conformance | Ensure constrained profiles never inherit full-graph or dynamic-allocation obligations accidentally | profile fixtures and negative examples |
| Backend SPI conformance | Ensure native, ROS2, MCU, gateway, and simulator backends expose compatible GRCL views | backend capability-query fixtures |
| ROS2 projection conformance | Ensure ROS2 adapter projects GRCL runtime participant state without claiming an rmw-level graph | adapter projection tests and negative assertions |
| SDK governance conformance | Ensure third-party SDKs declare manifests, permissions, resources, audit hooks, and quality level | manifest and permission policy fixtures |

## CI Stage Matrix

| Stage | Environment | Trigger | Required Before |
|---|---|---|---|
| `docs` | macOS or CI runner | every PR and local docs check | any merge |
| `schema-static` | Ubuntu container or CI runner | schema or fixture changes | schema acceptance |
| `c-abi-compile` | Ubuntu native container | `grcl-c` header changes | C ABI stabilization |
| `native-runtime` | Ubuntu native container | native runtime changes | native backend claims |
| `ros2-adapter` | Ubuntu ROS Humble/Jazzy container | ROS2 adapter changes | ROS2 backend claims |
| `mcu-profile-static` | MCU cross image or CI static runner | MCU profile/schema changes | MCU profile claims |
| `cross-language` | Ubuntu container with language runtimes | SDK wrapper changes | SDK stability claims |
| `conformance-suite` | CI matrix | release candidates | release or SDK governance claims |

The `docs` stage is the only stage that can run before implementation scaffolding exists. All other
stages are planned gates and must be introduced by accepted implementation plans.

## G5 Local Conformance Harness Baseline

G5 Conformance Harness v0.1 is the next planned milestone after M1. Its role is to convert the
accepted G1, G2, G3, and M1 artifacts into repeatable local drift checks before the project expands
into SDK wrappers, simulator runtime, ROS2 adapter work, MCU runtime work, management-plane
snapshots, or release hardening.

G5 is local and contract-focused:

- documentation conformance checks for links, unfinished markers, and status consistency.
- schema and fixture validation for runtime capability records, negotiation results, and MCU
  profiles.
- C ABI/header conformance checks for public header hygiene and C11/C++17 compile coverage.
- integration of the existing M1 runnable harness as a conformance stage.
- artifact-root enforcement for generated outputs.

G5 is not a CI rollout. It must not create GitHub Actions workflows, Dockerfiles, container images,
repo-wide build scripts, package-manager policy, SDK wrappers, pub/sub behavior, graph behavior,
transport behavior, ROS2 behavior, MCU runtime behavior, simulator behavior, management-plane
behavior, auth, remote management, event streams, or release-stability claims.

The goal-specific plan is
[G5 Conformance Harness Plan](../plans/2026-07-04-g5-conformance-harness-plan.md).

G5-G closeout verification completed the local harness. It used the refined non-goal scan from the
G5-G task brief so normal `grcl_runtime_*` identifiers and intentional conformance-checker pattern
definitions were not treated as pass/fail evidence. The G5 closeout passed independent audit.

## G6 Local SDK Boundary Drift Checks

G6 adds a narrow local stage to `scripts/run-conformance.sh` once the `grcl-cpp` and `grcl-py`
wrapper skeletons are present. The stage runs `scripts/check-sdk-boundaries.py`, writes its report
under `GRCL_PLATFORM_ARTIFACT_ROOT/g6/sdk-boundaries/`, and serves as a local drift guard for the
wrapper boundary. It is not a CI rollout, package-distribution decision, or claim that the SDKs are
fully stable.

The G6 drift check rejects backend-private or runtime-internal references from SDK wrapper source
files, rejects public references to `rcl`, `rmw`, `rclcpp`, socket APIs, pthread APIs, Docker, CI,
and repo-wide build-system declarations, verifies that `grcl-cpp` includes public `grcl/c/*.h`
headers instead of copying ABI declarations, and verifies that `grcl-py` routes through its
private native boundary rather than treating `grcl-cpp` as the semantic source.

## M3 Local Core Middleware Conformance

M3 adds a local core middleware stage to `scripts/run-conformance.sh` after the existing G5/G6
drift checks. The stage runs `examples/c/run_m3_examples.sh` only after the M3 C tests and C
examples pass independently. It validates the approved local contract for the in-process native
backend: pub/sub bytes delivery, service/client request/reply, executor pull dispatch,
runtime-local params, and deterministic cleanup.

M3 conformance is still local. It must not claim CI rollout, Docker support, package distribution,
release readiness, ROS2 behavior, transport interoperability, sockets, threads, distributed graph
behavior, distributed params, MCU runtime behavior, simulator behavior, management-plane behavior,
auth, remote management, event streams, SDK stability, or C++/Python example acceptance.

## M4 Local Contract Hardening Conformance

M4 adds a ninth local stage to `scripts/run-conformance.sh` after the M3 C examples stage. The
stage runs `src/grcl-c/tests/run_m4_contract_tests.sh` and hardens the completed M3 `grcl-c` local
contract with negative-path, ownership, messaging, executor, runtime-param, and capability
assertions.

M4 conformance remains local contract hardening only. It does not claim CI rollout, Docker
support, package distribution, release readiness, ROS2 or transport stability, simulator support,
MCU runtime stability, management-plane behavior, SDK stability, or any new runtime/backend/public
ABI surface beyond the already approved M3 local contract.

## M5 Local C++ Wrapper And Example Conformance

M5 adds a tenth local stage to `scripts/run-conformance.sh` after the M4 core contract stage. The
stage runs `examples/cpp/run_m5_cpp_examples.sh` only after the C++ wrappers, the dedicated M5 C++
test runner, and the standalone C++ examples pass independently. It validates that the approved
`grcl-cpp` wrapper surface remains a faithful wrapper over the native in-process local-core C
surface by exercising:

- pub/sub bytes delivery
- service/client request-reply forwarding with explicit `grcl_request_id_t` correlation
- runtime-local params set/get/list behavior
- one combined local-core scenario covering runtime, node, executor, pub/sub, service/client, and
  runtime-local params

M5 conformance remains local contract evidence only. It must not claim package distribution,
repo-wide build-system rollout, CI rollout, cross-platform installer support, ROS2 support,
transport interoperability, simulator behavior, MCU runtime behavior, management-plane behavior, or
Python SDK stability.

## Required Local Commands

Before any documentation-only architecture iteration is called complete:

```bash
git diff --check
python3 scripts/check-docs.py
scripts/run-conformance.sh
find . -maxdepth 1 \( -name build -o -name install -o -name log \) -print
```

`scripts/check-docs.py` is the durable local documentation conformance entrypoint. It
checks Markdown links in `README.md`, `AGENTS.md`, and `docs/**/*.md`, rejects unfinished durable
documentation markers, verifies that the three durable status documents agree on M1/G5/G6 execution
state, and writes its generated report only under `GRCL_PLATFORM_ARTIFACT_ROOT/g5/docs/` or the
workspace artifact root default.

`scripts/check-sdk-boundaries.py` is the G6 local SDK boundary drift checker. It scans the current
`grcl-cpp` and `grcl-py` wrapper skeleton surfaces for backend-private, runtime-internal, ROS2,
socket, threading, Docker, CI, package, and repo-wide build-system drift, and writes its generated
report under `GRCL_PLATFORM_ARTIFACT_ROOT/g6/sdk-boundaries/`.

`scripts/run-conformance.sh` is the top-level local runner. It may be invoked from the workspace
root as `src/grcl-platform/scripts/run-conformance.sh` or from the repository root as
`scripts/run-conformance.sh`. It preserves an existing `GRCL_PLATFORM_ARTIFACT_ROOT`, otherwise
defaults generated outputs to `/Users/aliben/Project/grcl-platform_ws/artifacts`, runs the ten
local stages in order, stops on the first failure, and keeps runner-owned summary metadata under
`GRCL_PLATFORM_ARTIFACT_ROOT/g5/conformance/`.

Current runner stages:

1. artifact root hygiene
2. documentation checks
3. runtime capability fixtures
4. MCU profile fixtures
5. C ABI/header checks
6. M1 runnable harness
7. SDK boundary drift checks
8. M3 core middleware examples
9. M4 core contract tests
10. M5 C++ local-core examples

## Artifact Ownership

CI and local test outputs must write under `GRCL_PLATFORM_ARTIFACT_ROOT`.

Local default:

```text
/Users/aliben/Project/grcl-platform_ws/artifacts
```

CI default:

```text
$RUNNER_TEMP/grcl-platform-artifacts
```

Repository-root `build/`, `install/`, `log/`, caches, virtual environments, wheels, and generated
reports are accidental pollution, not intended outputs.

## First Conformance Fixtures

The module skeleton implementation plan should introduce these fixtures before runtime behavior:

- `tests/conformance/runtime-capability/accepted.yaml`
- `tests/conformance/runtime-capability/degraded_accepted.yaml`
- `tests/conformance/runtime-capability/rejected_incompatible.yaml`
- `tests/conformance/mcu-profiles/baremetal-min.yaml`
- `tests/conformance/mcu-profiles/rtos-basic.yaml`
- `tests/conformance/mcu-profiles/rtos-posix-lite.yaml`

Each fixture must map to architecture fields in:

- `docs/architecture/runtime-capability-schema.md`
- `docs/architecture/mcu-profile-schema.md`
- `docs/architecture/runtime-capability-graph.md`

## Negative Conformance Requirements

Conformance must test unsupported or forbidden assumptions, not only valid paths:

- C public headers must not expose C++ or ROS2 types.
- SDK wrappers must not define independent lifecycle or ownership semantics.
- MCU profiles must not require full distributed graph caches.
- ROS2 adapter tests must not assume ROS2/rmw exposes a GRCL runtime participant graph.
- Docker and CI scripts must not write generated artifacts into the repository root by default.
- Capability, availability, and health records must not be collapsed into one object.

## Release Readiness Rule

A GRCL component cannot be called stable until its conformance layer exists and passes in the
appropriate environment. Documentation may describe planned behavior, but must not claim runtime,
SDK, backend, or ROS2 verification until the corresponding stage actually runs.
