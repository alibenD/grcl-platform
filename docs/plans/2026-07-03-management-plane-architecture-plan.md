# GRCL Platform Management Plane Architecture Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the next GRCL Platform architecture loop by scoping the management plane, recording
the dependency-ordered work, and stopping at explicit user decision gates without entering code
implementation.

**Architecture:** Treat the management plane as a cross-cutting architecture concern that consumes
runtime graph, capability, diagnostics, and SDK-governance evidence without redefining core
runtime semantics. Keep the work documentation-only until transport, authentication,
observability, and enforcement boundaries are selected.

**Tech Stack:** Markdown, repository architecture docs, ADRs, dependency analysis, status tracking.

---

## Scope

This plan is architecture-only. It does not authorize module skeletons, runtime headers, scripts,
Dockerfiles, or any implementation artifact.

## Task 1: Recover Remaining Management-Plane Gates

**Files:**
- Read: `docs/status/current-context.md`
- Read: `docs/status/goal-execution-queue.md`
- Read: `docs/architecture/runtime-layering.md`
- Read: `docs/architecture/backend-spi-contract.md`
- Read: `docs/architecture/runtime-capability-schema.md`
- Read: `docs/architecture/sdk-platform-governance.md`

- [ ] **Step 1: Confirm management plane is still architecture-only**

Verify that implementation remains paused and no code work is authorized.

- [ ] **Step 2: Inventory remaining design gates**

List every place where transport, authentication, event schema, permission mapping, or release
policy remains explicitly undecided.

## Task 2: Write Durable Scope Document

**Files:**
- Create: `docs/architecture/management-plane-decision-scope.md`

- [ ] **Step 1: Record why this is the next architecture loop**

Link the scope choice back to explicit remaining gates in the accepted architecture set.

- [ ] **Step 2: Record non-goals and already-fixed upstream constraints**

Prevent future sessions from turning this loop into hidden implementation work.

- [ ] **Step 3: Record user decision options**

Describe the decision space without choosing transport, authentication, eventing, or enforcement
 policy on behalf of the user.

## Task 3: Update Durable Recovery State

**Files:**
- Modify: `docs/status/current-context.md`
- Modify: `docs/status/goal-execution-queue.md`
- Modify: `docs/README.md`
- Modify: `README.md`

- [ ] **Step 1: Register this plan and scope doc as current entry points**

Future sessions should discover the new architecture loop from repository documents.

- [ ] **Step 2: Replace the active goal queue with the new architecture loop**

Carry forward the blocked module-skeleton gate as context, but make the management-plane loop the
active pre-implementation goal.

- [ ] **Step 3: Preserve the implementation pause**

The updated status docs must still prohibit runtime, SDK, Docker, script, and module-skeleton
implementation without user approval.

## Task 4: Stop At User Decision Gate

**Files:**
- Read: `docs/architecture/management-plane-decision-scope.md`
- Read: `docs/status/goal-execution-queue.md`

- [ ] **Step 1: Present explicit decision options**

Stop once the repository documents clearly show which management-plane choices still require user
direction.

- [ ] **Step 2: Do not author the management-plane ADR yet**

Do not collapse undecided product or engineering direction into a fake resolved architecture
document.

## Task 5: Validate Documentation Hygiene

**Files:**
- Read: `docs/architecture/management-plane-decision-scope.md`
- Read: `docs/plans/2026-07-03-management-plane-architecture-plan.md`
- Read: `docs/status/current-context.md`
- Read: `docs/status/goal-execution-queue.md`

- [ ] **Step 1: Check diff hygiene**

Run:

```bash
git diff --check
```

Expected: no output and exit code 0.

- [ ] **Step 2: Check document inventory**

Run:

```bash
find docs -name '*.md' -print | sort
```

Expected: the new management-plane scope document and plan are present.

- [ ] **Step 3: Stop without implementation**

Do not create code, scripts, headers, Dockerfiles, schemas, or module directories as part of this
loop.
