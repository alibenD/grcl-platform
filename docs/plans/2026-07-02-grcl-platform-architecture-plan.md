# GRCL Platform Architecture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish the GRCL Platform architecture baseline and provide a reviewable implementation
roadmap; module implementation starts only after user confirmation.

**Architecture:** Use `grcl-platform` as a modular monorepo during architecture stabilization.
Keep `grcl-c` as the core contract, keep language SDKs over `grcl-c`, and require runtime
capability exchange before cross-runtime communication.

**Tech Stack:** Markdown, Mermaid, Git, future C/C++/Python/ROS2/RTOS module skeletons, future
Docker/Ubuntu/ROS Humble development environment.

---

## Scope

This plan is intentionally architecture-first. It does not authorize runtime, SDK, or backend code
implementation until the user reviews and accepts the plan.

## Task 1: Review Architecture Baseline

**Files:**
- Read: `docs/architecture/system-overview.md`
- Read: `docs/architecture/repository-topology.md`
- Read: `docs/architecture/runtime-layering.md`
- Read: `docs/architecture/dependency-analysis.md`

- [ ] **Step 1: Verify system-level boundaries**

Confirm that `grcl-platform` is the system-level source of truth and that the existing
`github.com/alibenD/grcl` repository is treated as a future `grcl-cpp` candidate.

- [ ] **Step 2: Verify module split**

Confirm that `grcl-c`, `grcl-cpp`, `grcl-py`, `grcl-idl`, `grcl-runtime-native`,
`grcl-runtime-ros2`, `grcl-runtime-mcu`, `grcl-gateway`, and `grcl-tools` are the initial module
set.

- [ ] **Step 3: Update topology if user changes direction**

If the user prefers multi-repo or a different module split, update `docs/adr/ADR-0001-*` and
`docs/architecture/repository-topology.md` before any implementation work.

## Task 2: Review GRCL-C Contract Design

**Files:**
- Read: `docs/architecture/grcl-c-contract-boundary.md`
- Read: `docs/architecture/grcl-c-api-shape.md`
- Read: `docs/adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md`

- [ ] **Step 1: Validate the first handle set**

Confirm that the initial `grcl-c` handle set covers runtime, node, endpoint, publisher,
subscription, executor, allocator, storage, type support, backend, and transport.

- [ ] **Step 2: Validate lifecycle and ownership**

Confirm create/destroy, init-with-storage, controlled-storage, and ownership rules are sufficient
for first implementation.

- [ ] **Step 3: Validate error and result model**

Confirm result categories separate caller, resource, compatibility, runtime, and graph/session
errors.

- [ ] **Step 4: Validate ABI versioning**

Confirm struct-size and versioning rules are acceptable before writing headers.

## Task 3: Review Runtime Capability Schema

**Files:**
- Read: `docs/architecture/runtime-capability-graph.md`
- Read: `docs/architecture/runtime-capability-schema.md`
- Read: `docs/adr/ADR-0006-runtime-capability-schema.md`

- [ ] **Step 1: Validate `RuntimeCapabilityRecord`**

Confirm protocol, transport, QoS, encoding, memory, executor, graph, diagnostics, and security
sections are sufficient for cross-runtime negotiation.

- [ ] **Step 2: Validate `RuntimeAvailabilityRecord`**

Confirm availability fields stay separate from stable capability fields.

- [ ] **Step 3: Validate `RuntimeHealthRecord`**

Confirm health states cover lease, heartbeat, degraded, reconnecting, recovered, and faulted
behavior.

- [ ] **Step 4: Validate negotiation result schema**

Confirm `accepted`, `degraded_accepted`, and `rejected_incompatible` payloads expose effective
capability, reasons, downgrade suggestion, and diagnostic code.

## Task 4: Review MCU Runtime Profile Design

**Files:**
- Read: `docs/architecture/mcu-runtime-profiles.md`
- Read: `docs/architecture/mcu-profile-schema.md`
- Read: `docs/adr/ADR-0007-mcu-runtime-profile-contract.md`

- [ ] **Step 1: Validate `baremetal-min`**

Define required graph, memory, transport, executor, diagnostics, and unsupported capabilities.

- [ ] **Step 2: Validate `rtos-basic`**

Define FreeRTOS-like requirements and extension points.

- [ ] **Step 3: Validate `rtos-posix-lite`**

Define NuttX/Zephyr-like POSIX subset expectations.

- [ ] **Step 4: Validate Linux/gateway/sim profiles**

Define `linux-full`, `gateway-full`, and `sim-full` obligations.

## Task 5: Review Backend SPI Design

**Files:**
- Read: `docs/architecture/backend-spi-contract.md`
- Read: `docs/adr/ADR-0008-backend-spi-contract.md`

- [ ] **Step 1: Define backend responsibilities**

Define runtime, communication, graph, capability, diagnostics, and management responsibilities.

- [ ] **Step 2: Define backend capability query**

Define how backends expose supported subsets to `grcl-c`.

- [ ] **Step 3: Define ROS2 projection requirement**

Define how ROS2 adapter backend projects GRCL runtime participant state without assuming ROS2
already exposes it.

## Task 6: Review Language SDK Design

**Files:**
- Read: `docs/architecture/language-sdk-strategy.md`
- Read: `docs/adr/ADR-0009-language-sdk-binding-contract.md`

- [ ] **Step 1: Define C++ SDK wrapper policy**

Define RAII, templates, exceptions policy, allocator integration, and direct `grcl-c` ownership.

- [ ] **Step 2: Define Python SDK binding policy**

Define direct C binding or private native shim over `grcl-c`, not public `grcl-cpp` binding.

- [ ] **Step 3: Define cross-language conformance tests**

Define test scenarios that compare C, C++, and Python behavior through the same core contract.

## Task 7: Review Development Environment Design

**Files:**
- Read: `docs/architecture/development-environment.md`
- Read: `docker/README.md`
- Read: `scripts/README.md`

- [ ] **Step 1: Define macOS host role**

Define macOS as editor/orchestration host, not the primary ROS2 validation runtime.

- [ ] **Step 2: Define Ubuntu/ROS container role**

Define Ubuntu + ROS Humble/Jazzy container verification paths.

- [ ] **Step 3: Define future CI matrix**

Define native Linux, ROS2 backend, docs checks, conformance checks, and future MCU cross builds.

## Task 8: Produce User Review Package

**Files:**
- Read: `docs/README.md`
- Read: `README.md`
- Read: `docs/architecture/dependency-analysis.md`
- Read: this plan file

- [ ] **Step 1: Ensure all entry points link correctly**

Run:

```bash
find docs -name '*.md' -print | sort
```

Expected: all architecture, ADR, and plan files are present.

- [ ] **Step 2: Check markdown diff hygiene**

Run:

```bash
git diff --check
```

Expected: no output and exit code 0.

- [ ] **Step 3: Stop for user review**

Do not implement module code until the user confirms the architecture plan.

## Post-Review Implementation Sequence

The following sequence is blocked until user confirmation:

1. Create module skeleton directories and module-local README files.
2. Add `grcl-c` public header skeletons from `grcl-c-api-shape.md`.
3. Add schema test vectors for runtime capability records.
4. Add MCU profile descriptor examples and validators.
5. Add backend SPI header/function-table design.
6. Add language SDK wrapper skeletons that compile against `grcl-c`.
7. Add Docker/Ubuntu verification environment.
8. Add conformance test harness.
