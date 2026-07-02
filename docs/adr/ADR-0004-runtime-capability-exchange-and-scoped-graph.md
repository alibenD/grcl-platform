# ADR-0004 Runtime Capability Exchange And Scoped Graph

- Status: `accepted`
- Date: `2026-07-02`

## Decision

GRCL adopts a dual-layer graph model:

1. Runtime Participant Graph
2. Application Node/Endpoint Graph

Runtime-level discovery and capability exchange are mandatory for cross-runtime communication.
Node and endpoint discovery remain the primary application-level model exposed to developers.
Resource-constrained runtimes are not required to maintain the full distributed graph.

## Corrected ROS2 Baseline

ROS2 exposes a node-level graph to users and tools. `rcl` provides graph access and graph events for
client libraries, and it reaches concrete middleware implementations through `rmw`. GRCL must not
claim that `rmw` already exposes the runtime participant graph proposed here.

DDS/RTPS implementations have participant discovery and endpoint discovery underneath, but GRCL's
runtime participant graph is a GRCL product abstraction for heterogeneous robot deployments.

## Options Considered

- Node-only discovery: rejected because runtime/platform capability is discovered too late.
- Full global graph on every runtime: rejected because it is too expensive for MCU/RTOS targets.
- Runtime participant graph plus scoped application graph: accepted because it provides early
  capability negotiation while keeping constrained runtimes lightweight.

## Required Concepts

- `RuntimeParticipant`
- `RuntimeCapabilityRecord`
- `RuntimeAvailabilityRecord`
- `RuntimeHealthRecord`
- `RuntimeSession`
- `RuntimeRoute`
- `GraphProjectionMode`
- `CapabilityNegotiationResult`

## Projection Modes

- `full`
- `runtime-only`
- `interest-scoped`
- `static-manifest`
- `gateway-represented`

## Consequences

The platform must define capability schema, graph projection rules, session negotiation, and
management visibility before implementing cross-runtime communication.

