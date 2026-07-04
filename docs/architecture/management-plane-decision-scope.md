# Management Plane Decision Scope

## Purpose

This document scopes the next GRCL Platform architecture closure after the initial baseline
documents. It does not authorize implementation. Its role is to collect the already-decided
upstream constraints, identify the remaining management-plane decisions, and present the user
decision gates that block a full management-plane ADR.

## Why This Is The Next Architecture Loop

Multiple accepted architecture documents defer management-plane details:

- [Runtime Layering](runtime-layering.md) leaves management API transport and authentication for a
  later ADR.
- [Backend SPI Contract](backend-spi-contract.md) leaves the management event schema open.
- [Runtime Capability Schema](runtime-capability-schema.md) treats management API output as a future
  consumer of capability, availability, and health records.
- [SDK Platform Governance](sdk-platform-governance.md) leaves permission language, enforcement
  points, and registry/provenance details open.

This makes the management plane the clearest remaining architecture loop that can advance without
crossing into runtime, SDK, build, or module-skeleton implementation.

## Concept-First Refinement

User review showed that the original decision areas were too abstract to choose directly. The next
step is therefore [Management Plane Concept And Scope](management-plane-concept-and-scope.md), which
defines the management plane as a read-only observation contract and snapshot model before any
management-plane ADR is written.

The concept-first path changes the immediate gate:

- First review the management-plane engineering shape, snapshot model, data sources, and deferrals.
- Then decide whether to promote the concept into a management-plane ADR.
- Only after that should the project decide concrete transport, authentication, event-stream, or
  SDK permission-policy details.

## Upstream Decisions Already Fixed

- `grcl-platform` remains the top-level architecture repository.
- Code implementation remains paused until explicit user approval.
- `grcl-c` is the core contract boundary.
- Language SDKs must not define independent runtime semantics.
- Runtime capability exchange and scoped graph projection are first-class baseline decisions.
- MCU and RTOS targets implement declared profile subsets rather than full GRCL behavior by
  default.
- Workspace artifact isolation and container verification policy are already defined.

## Management Plane Must Observe

The management plane must be able to consume or expose:

- runtime graph state
- node graph state
- capability negotiation results
- runtime capability, availability, and health records
- backend diagnostics and degraded-state information
- SDK governance evidence where policy later requires it

The management plane must not redefine runtime semantics that belong to `grcl-c`, backend SPI, or
profile contracts.

## Non-Goals For This Loop

- Do not implement management APIs, transports, or auth code.
- Do not create module skeletons, scripts, Dockerfiles, or runtime headers.
- Do not decide release-governance details beyond what is needed to scope future ADRs.
- Do not migrate the external `github.com/alibenD/grcl` repository during this loop.

## Remaining Decision Areas

The following areas remain future ADR decisions. They should not be selected until the concept
model has been reviewed.

### 1. Management Transport Boundary

What transport shapes are part of the first management-plane baseline?

Options to decide later:

- `local-only`: standardize only an in-process or same-host management surface for the first ADR.
- `backend-native`: allow each backend family to expose a profile-appropriate management channel
  under one logical schema.
- `networked-control-plane`: define a remote transport baseline now, likely through gateway/native
  paths, with explicit compatibility and security rules.

### 2. Authentication And Authorization Boundary

What security posture is required for the first management surface?

Options to decide later:

- `trusted-internal`: management plane is internal/trusted only in the first baseline.
- `mutual-auth`: define authenticated runtime-to-tool or tool-to-gateway sessions in the first ADR.
- `tiered-security`: profile-dependent security, where Linux/gateway paths require stronger auth
  while some MCU profiles permit reduced or absent management auth.

### 3. Snapshot Versus Event Stream Contract

How much observability is required in the first management baseline?

Options to decide later:

- `snapshot-only`: require point-in-time reads first and defer event streams.
- `snapshot-plus-events`: require both snapshot and event stream semantics in the first ADR.
- `profile-tiered`: require snapshots everywhere, but allow event streams only on profiles that
  declare support.

### 4. SDK Governance Enforcement Mapping

Where will future SDK manifest and permission enforcement hook into the platform?

Options to decide later:

- `management-only`: management plane evaluates manifests and permissions, while core/runtime stays
  policy-agnostic.
- `core-and-management`: core contract exposes enforcement hooks while management plane supplies
  policy input and audit visibility.
- `backend-assisted`: backend SPI reports enforcement-relevant capabilities and the management plane
  coordinates policy decisions across backends.

### 5. External Prototype Relationship

How should the current external `github.com/alibenD/grcl` repository relate to future management
 surfaces and observability work?

Options to decide later:

- `leave-external`: keep it external until `grcl-c` and management contracts are more mature.
- `mirror-contracts`: reflect selected architecture contracts there without migrating the repo.
- `prepare-migration`: design management-plane assumptions with eventual `grcl-cpp` migration in
  mind, but do not migrate yet.

## Recommended Architecture Work Before User Decision

The following work does not require choosing the options above:

1. Create a dedicated management-plane architecture plan.
2. Update durable status files so future sessions recover this loop from repository documents.
3. Record the new user decision gate in the goal execution queue.

## User Decision Gate

The next gate is review of
[Management Plane Concept And Scope](management-plane-concept-and-scope.md). The project should not
write a resolved management-plane ADR until the user accepts or revises that concept model.
