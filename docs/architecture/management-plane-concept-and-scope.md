# Management Plane Concept And Scope

## Purpose

This document makes the GRCL management plane concrete before the project writes a management-plane
ADR. It explains what the management plane is, what it is not, what the first version should expose,
where the data comes from, and which concerns are intentionally deferred.

This is an architecture concept document only. It does not authorize runtime code, SDK code, build
scripts, Dockerfiles, module skeletons, remote protocols, or repository migration.

## Working Definition

The management plane is the public observation and management contract for GRCL platform state.

It is not just one protocol and it is not just one data structure. It is the stable contract that
tools, conformance tests, dashboards, gateways, and future SDK tooling can use to understand:

- which runtimes exist.
- which nodes, endpoints, publishers, subscriptions, services, and clients are visible.
- which capabilities each runtime and backend declares.
- which capability negotiations succeeded, degraded, or failed.
- which runtimes are ready, degraded, faulted, or stopped.
- which diagnostics and profile limits are visible to tools.

The management plane must not redefine runtime semantics. Runtime lifecycle, object ownership,
backend SPI behavior, graph semantics, and profile limits remain owned by `grcl-c`, backend SPI, and
profile contracts.

## Engineering Shape Over Time

The management plane can appear as several engineering artifacts over time:

| Phase | Entity Shape | Purpose |
|---|---|---|
| Architecture concept | docs and diagrams | Define what is observable and what remains out of scope. |
| Contract design | snapshot schema and ADR | Define stable fields, sources, and profile requirements. |
| `grcl-c` contract | status structs or query API | Let core runtime state be read through a stable contract. |
| Backend SPI obligation | backend-provided snapshot fields | Let native, ROS2, MCU, gateway, and simulator backends expose compatible state. |
| Tooling | `grcl-tools` CLI JSON/YAML output | Let humans and CI inspect runtime, graph, capability, and diagnostics state. |
| Future management service | daemon or gateway endpoint | Optional remote or cross-process access after security and transport ADRs. |

The first version should focus on the contract design phase: a read-only observation contract and a
snapshot model. It should not start with a remote management protocol.

## Conceptual Placement

```text
Applications / SDK users
  -> grcl-cpp / grcl-py / future SDKs
    -> grcl-c core contract
      -> runtime host and backend SPI
        -> native / ROS2 / MCU / gateway / simulator backends

Management plane
  observes grcl-c-visible runtime state, backend-provided state, profile declarations,
  capability negotiation results, health records, and diagnostics.

Tools / CI / dashboard / gateway operations
  consume the management-plane contract without depending on backend internals.
```

The management plane sits beside the runtime path. It observes and reports platform state; it is not
the message transport, executor, ROS2 adapter, MCU runtime, or SDK user API.

## Version 1 Scope

The first management-plane architecture scope is:

- read-only observation contract.
- status snapshot model.
- clear field ownership between `grcl-c`, backend SPI, profile declarations, and future SDK
  governance.
- future tooling shape for CLI, CI, dashboard, gateway diagnostics, and bug reports.

Version 1 explicitly does not include:

- authentication or authorization mechanism.
- remote control plane or remote management protocol.
- mandatory event stream semantics.
- complete SDK permission policy language.
- public SDK marketplace, registry, signing, or provenance system.
- migration of `github.com/alibenD/grcl` into this repository.

## Management Snapshot

A management snapshot is a structured read of platform state at one point in time. It answers:

```text
What does the platform know about this runtime, graph, capability set, health state,
and diagnostic condition right now?
```

It is different from an event stream. A snapshot can be queried, serialized, attached to bug
reports, checked in CI, or shown by a CLI. Event streams can be added later for runtimes and
profiles that declare support.

## Snapshot Content Model

| Section | Example Contents | Primary Source |
|---|---|---|
| runtime identity | runtime id, boot id, domain, profile, backend family, implementation version | `grcl-c` and backend SPI |
| lifecycle state | created, starting, ready, degraded, faulted, stopped | `grcl-c` runtime lifecycle |
| graph state | visible nodes, endpoints, publishers, subscriptions, services, clients | `grcl-c` graph state and backend graph projection |
| capability summary | transports, QoS subset, memory limits, executor model, graph projection mode | runtime capability records and backend SPI |
| availability summary | free slots, buffer pressure, throttles, active degradation | backend SPI when profile supports dynamic availability |
| health state | heartbeat, lease, state, reason, scope | runtime health record |
| negotiation result | accepted, degraded accepted, rejected incompatible, reasons, effective capability | capability negotiation contract |
| diagnostics summary | recent errors, warnings, counters, backend degraded reason | backend diagnostics and profile-supported metrics |
| SDK governance evidence | manifest presence, declared permissions, audit level | future SDK governance, not required for v1 |

The snapshot schema should make unsupported fields explicit. For example, a `baremetal-min` profile
may expose static manifest and basic health while omitting dynamic availability fields.

## Data Ownership

| Owner | Owns | Does Not Own |
|---|---|---|
| `grcl-c` | core lifecycle, handles, result categories, capability query boundary, visible object ownership | backend-private scheduling, transport internals, remote auth policy |
| Backend SPI | backend profile, graph projection, diagnostics, availability, backend capability details | public SDK semantics, core lifecycle rules |
| Profile contracts | required, optional, and unsupported capabilities for target classes | implementation-specific runtime behavior |
| Management plane | stable observation contract and tool-consumable view | runtime semantics, transport implementation, executor behavior |
| Future SDK governance | manifest, permission, audit, and quality-level evidence | first-version runtime behavior or full policy enforcement |

## Expected Uses

| Use Case | Snapshot Value |
|---|---|
| CLI debugging | Show which runtimes, nodes, endpoints, and capabilities are currently visible. |
| Conformance tests | Assert that a runtime/profile reports required fields and omits unsupported ones clearly. |
| Dashboard | Render runtime health, degraded state, graph visibility, and capability negotiation results. |
| Gateway observation | Let a gateway represent downstream MCU runtimes without pretending they support full graph state. |
| Bug report | Attach one structured state dump instead of relying only on logs. |
| Capability debugging | Explain why a runtime pair accepted, degraded, or rejected communication. |

## SDK Permission Relationship

Future SDK permission work is related to `grcl-c` because SDKs eventually request core actions such
as creating nodes, creating publishers, opening channels, or calling services. If GRCL later supports
third-party control or planning extensions, permission checks cannot live only in a dashboard or
documentation layer.

This document does not design the permission system. It only records the future architecture
constraint:

- management plane can provide policy visibility, manifest evidence, and audit views.
- `grcl-c` may need enforcement points at object creation, channel opening, or service-call
  boundaries.
- backend SPI may need to report backend-specific limits and enforcement-relevant capabilities.
- the policy language, authorization model, and enforcement behavior require a later ADR.

## Authentication And Remote Access Deferral

Authentication and authorization are intentionally out of scope for version 1. The first management
contract should assume local or trusted development access and read-only observation.

Future remote management, cross-machine control, third-party SDK control, or fleet operations must
be preceded by a dedicated authentication and authorization ADR. That later ADR should choose the
security model after the core runtime, snapshot model, and management data boundaries are stable.

## Event Stream Deferral

Version 1 should define snapshots first. Event streams are useful but add ordering, loss, replay,
reconnect, backpressure, and profile-limit questions. A later ADR or contract revision can define
event streams after snapshot fields and profile obligations are accepted.

Profiles can still declare future event-stream capability, but the first concept baseline should
not require every profile to stream management events.

## External `grcl` Repository

The existing `github.com/alibenD/grcl` repository does not participate in this management-plane
design loop. It remains an external C++/ROS2 prototype and a future `grcl-cpp` candidate.

Future `grcl-cpp` may inherit API and product experience from that repository, but its long-term
architecture is a C++ SDK over `grcl-c`, not a separate pure-C++ source of runtime semantics.

## ADR Entry Conditions

A management-plane ADR should be written only after this concept model is reviewed. The ADR should
then decide:

- the exact name and boundary of the first management contract.
- the first snapshot schema shape.
- which snapshot fields are required, optional, unsupported, or profile-dependent.
- how snapshot data maps to `grcl-c`, backend SPI, profile contracts, and future tools.
- which remote access, authentication, event stream, and SDK permission concerns remain deferred.

Until then, the project should not treat transport, authentication, eventing, or SDK permission
policy as resolved decisions.
