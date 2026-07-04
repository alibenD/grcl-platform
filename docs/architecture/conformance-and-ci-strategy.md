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

## Required Local Commands

Before any documentation-only architecture iteration is called complete:

```bash
git diff --check
python3 scripts/check-docs.py
find . -maxdepth 1 \( -name build -o -name install -o -name log \) -print
```

`scripts/check-docs.py` is the durable local documentation conformance entrypoint for G5-B. It
checks Markdown links in `README.md`, `AGENTS.md`, and `docs/**/*.md`, rejects unfinished durable
documentation markers, verifies that the three durable status documents agree on M1/G5 execution
state, and writes its generated report only under `GRCL_PLATFORM_ARTIFACT_ROOT/g5/docs/` or the
workspace artifact root default.

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
