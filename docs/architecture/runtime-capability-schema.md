# Runtime Capability Schema

## Purpose

This document defines the first schema-level design for runtime capability exchange. It is the
input to future C structs, wire protocol frames, management API output, and conformance tests.

## Record Separation

The runtime graph uses three records with different update behavior.

| Record | Meaning | Update Frequency |
|---|---|---|
| `RuntimeCapabilityRecord` | what the runtime can support in principle | stable or versioned |
| `RuntimeAvailabilityRecord` | what resources are currently available | runtime-dependent |
| `RuntimeHealthRecord` | whether runtime/session/channel is healthy | heartbeat-driven |

## RuntimeCapabilityRecord

Required sections:

| Section | Required Fields |
|---|---|
| identity | `runtime_id`, `boot_id`, `domain`, `profile`, `implementation_id`, `implementation_version` |
| protocol | `grcl_protocol_major`, `grcl_protocol_minor`, `capability_schema_version` |
| transport | supported transports, max MTU, fragmentation, retransmission, flow control |
| qos | best-effort, reliable, history max, deadline, liveliness, durability subset |
| encoding | CDR-compatible encoding, static type support, dynamic type support, endian support, compression |
| memory | max payload, max entities, static storage, no heap after init, history bytes |
| executor | poll, sequential, trigger any/all, LET-like behavior, multithread support |
| graph | projection modes, dynamic endpoint delta, static manifest, full graph cache |
| diagnostics | basic health, extended metrics, tracing, remote log sink |
| security | authentication, authorization, enclave, audit, signed manifest |

## RuntimeAvailabilityRecord

Required sections:

| Section | Required Fields |
|---|---|
| resources | free channels, free endpoint slots, free message buffers |
| pressure | memory pressure, queue pressure, transport pressure |
| rate limits | current publish limit, diagnostics limit, graph event limit |
| degradation | active throttles, active drops, active fallback modes |

Availability fields must not be required on `baremetal-min` unless the profile explicitly supports
dynamic availability reporting.

## RuntimeHealthRecord

Required sections:

| Section | Required Fields |
|---|---|
| lease | lease duration, last heartbeat, expiration time |
| state | `starting`, `ready`, `degraded`, `reconnecting`, `recovered`, `faulted`, `stopped` |
| reason | standard diagnostic code and optional human-readable summary |
| scope | runtime, session, channel, endpoint |

## GraphProjectionMode

| Mode | Required Capability |
|---|---|
| `full` | graph snapshot and delta event stream |
| `runtime-only` | runtime identity, lease, health, capability summary |
| `interest-scoped` | interest registration and filtered endpoint deltas |
| `static-manifest` | manifest hash and predeclared endpoint set |
| `gateway-represented` | upstream representation and downstream runtime identity mapping |

## CapabilityNegotiationResult

Required fields:

- `status`: `accepted`, `degraded_accepted`, or `rejected_incompatible`
- `effective_capability`: capability set that will actually be used
- `rejection_reasons`: machine-readable reason list
- `suggested_downgrade`: optional lower capability request
- `diagnostic_code`: standard diagnostic code
- `scope`: session, channel, endpoint, or graph projection

## Compatibility Rules

- Major protocol mismatch is rejected.
- Minor protocol mismatch may negotiate down only when the older peer supports all required fields.
- Unsupported required capability is rejected.
- Unsupported optional capability can be omitted only when the caller explicitly marked it optional.
- Downgrade is visible in graph state and diagnostics.

