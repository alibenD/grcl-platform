# GRCL Platform Architecture Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish the GRCL Platform architecture baseline, then implement modules only after user
review confirms the architecture and dependency plan.

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

## Task 2: Complete GRCL-C Contract Design

**Files:**
- Modify: `docs/architecture/grcl-c-contract-boundary.md`
- Create: `docs/architecture/grcl-c-api-shape.md`
- Create: `docs/adr/ADR-0005-grcl-c-handle-and-lifecycle-contract.md`

- [ ] **Step 1: Define the first handle set**

Define the initial `grcl-c` handle set for runtime, node, endpoint, publisher, subscription,
executor, allocator, storage, type support, backend, and transport.

- [ ] **Step 2: Define lifecycle and ownership**

Define create/destroy, init/fini, controlled-storage, and ownership rules.

- [ ] **Step 3: Define error and result model**

Define result code categories and diagnostic propagation.

- [ ] **Step 4: Define ABI versioning**

Define options struct size/version rules and compatibility expectations.

## Task 3: Complete Runtime Capability Schema

**Files:**
- Modify: `docs/architecture/runtime-capability-graph.md`
- Create: `docs/architecture/runtime-capability-schema.md`
- Create: `docs/adr/ADR-0006-runtime-capability-schema.md`

- [ ] **Step 1: Define `RuntimeCapabilityRecord`**

Define protocol, transport, QoS, encoding, memory, executor, graph, diagnostics, and security
capability sections.

- [ ] **Step 2: Define `RuntimeAvailabilityRecord`**

Define resource availability fields that can change at runtime.

- [ ] **Step 3: Define `RuntimeHealthRecord`**

Define lease, heartbeat, degraded, reconnecting, recovered, and faulted states.

- [ ] **Step 4: Define negotiation result schema**

Define `accepted`, `degraded_accepted`, and `rejected_incompatible` payloads.

## Task 4: Complete MCU Runtime Profile Design

**Files:**
- Modify: `docs/architecture/mcu-runtime-profiles.md`
- Create: `docs/architecture/mcu-profile-schema.md`
- Create: `docs/adr/ADR-0007-mcu-runtime-profile-contract.md`

- [ ] **Step 1: Define `baremetal-min`**

Define required graph, memory, transport, executor, diagnostics, and unsupported capabilities.

- [ ] **Step 2: Define `rtos-basic`**

Define FreeRTOS-like requirements and extension points.

- [ ] **Step 3: Define `rtos-posix-lite`**

Define NuttX/Zephyr-like POSIX subset expectations.

- [ ] **Step 4: Define Linux/gateway/sim profiles**

Define `linux-full`, `gateway-full`, and `sim-full` obligations.

## Task 5: Complete Backend SPI Design

**Files:**
- Create: `docs/architecture/backend-spi-contract.md`
- Create: `docs/adr/ADR-0008-backend-spi-contract.md`

- [ ] **Step 1: Define backend responsibilities**

Define runtime, communication, graph, capability, diagnostics, and management responsibilities.

- [ ] **Step 2: Define backend capability query**

Define how backends expose supported subsets to `grcl-c`.

- [ ] **Step 3: Define ROS2 projection requirement**

Define how ROS2 adapter backend projects GRCL runtime participant state without assuming ROS2
already exposes it.

## Task 6: Complete Language SDK Design

**Files:**
- Create: `docs/architecture/language-sdk-strategy.md`
- Create: `docs/adr/ADR-0009-language-sdk-binding-contract.md`

- [ ] **Step 1: Define C++ SDK wrapper policy**

Define RAII, templates, exceptions policy, allocator integration, and direct `grcl-c` ownership.

- [ ] **Step 2: Define Python SDK binding policy**

Define direct C binding or private native shim over `grcl-c`, not public `grcl-cpp` binding.

- [ ] **Step 3: Define cross-language conformance tests**

Define test scenarios that compare C, C++, and Python behavior through the same core contract.

## Task 7: Complete Development Environment Design

**Files:**
- Create: `docs/architecture/development-environment.md`
- Create: `docker/README.md`
- Create: `scripts/README.md`

- [ ] **Step 1: Define macOS host role**

Define macOS as editor/orchestration host, not the primary ROS2 validation runtime.

- [ ] **Step 2: Define Ubuntu/ROS container role**

Define Ubuntu + ROS Humble/Jazzy container verification paths.

- [ ] **Step 3: Define future CI matrix**

Define native Linux, ROS2 backend, docs checks, conformance checks, and future MCU cross builds.

## Task 8: Produce User Review Package

**Files:**
- Modify: `docs/README.md`
- Modify: `README.md`
- Modify: `docs/architecture/dependency-analysis.md`
- Modify: this plan file

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

