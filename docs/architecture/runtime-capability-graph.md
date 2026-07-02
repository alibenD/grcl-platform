# Runtime Capability Graph

## Purpose

The runtime capability graph lets heterogeneous runtimes understand whether peers can support
required protocol, transport, QoS, encoding, memory, executor, graph, diagnostics, and security
capabilities before channels are opened.

## Dual Graph Model

```text
Runtime Participant Graph
  RuntimeParticipant
  RuntimeCapabilityRecord
  RuntimeAvailabilityRecord
  RuntimeHealthRecord
  RuntimeSession
  RuntimeRoute
  GraphProjectionMode
  CapabilityNegotiationResult

Application Node/Endpoint Graph
  Node
  Endpoint
  Topic
  Service
  Action
  Channel
  Type
  QoS
```

## Rules

- Runtime participant discovery precedes node/endpoint discovery across runtimes.
- Capability exchange precedes channel establishment.
- Node and endpoint discovery remain the application-level model.
- MCU/RTOS runtimes are not required to maintain a full global graph.
- Capability, availability, and health are separate records.
- Silent downgrade is not allowed.

## Projection Modes

| Mode | Meaning |
|---|---|
| `full` | full runtime/node/endpoint graph |
| `runtime-only` | peer identity, lease, health, capability summary |
| `interest-scoped` | graph items related to interests or active channels |
| `static-manifest` | manifest hash or predeclared endpoints |
| `gateway-represented` | gateway publishes downstream runtime representation |

## Diagram

See [runtime-graph.mmd](../assets/runtime-graph.mmd).

