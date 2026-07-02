# MCU Profile Schema

## Purpose

This document defines the schema used to describe MCU/RTOS runtime profiles. It is the input to
future `grcl-c` controlled storage, backend SPI capability checks, conformance tests, and
development-environment targets.

## Profile Descriptor

Each profile descriptor must include:

| Field | Meaning |
|---|---|
| `profile_name` | stable profile identifier |
| `target_class` | bare metal, RTOS, POSIX-lite RTOS, Linux, gateway, simulator |
| `allocation_policy` | heap allowed, init-only heap, caller-provided storage only |
| `graph_policy` | full, runtime-only, interest-scoped, static-manifest, gateway-represented |
| `transport_policy` | supported transports and max MTU |
| `qos_policy` | supported QoS subset |
| `executor_policy` | poll, sequential, trigger, LET-like, multithread |
| `diagnostics_policy` | none, basic, extended, tracing |
| `security_policy` | none, link-auth, signed manifest, audit |
| `entity_limits` | max nodes, endpoints, sessions, channels, timers |
| `payload_limits` | max payload size and history bytes |

## Profile: baremetal-min

Required:

- caller-provided storage
- no heap after init
- static manifest or local graph only
- runtime-only peer summary
- poll or externally driven executor
- best-effort small payload channel
- basic heartbeat or gateway-observed health

Forbidden:

- full graph cache
- dynamic endpoint discovery storm
- hidden heap allocation after init
- unbounded queues
- runtime plugin loading

## Profile: rtos-basic

Required:

- static pools
- bounded executor state
- local graph
- session-scoped peer summary
- optional interest-scoped endpoint delta
- transport adapter for FreeRTOS-like systems
- explicit task/queue ownership policy

Allowed:

- limited reliable stream if configured history and buffers are bounded
- basic diagnostics
- gateway representation

## Profile: rtos-posix-lite

Required:

- RTOS adapter with POSIX-like clock, mutex, task/thread, and socket-like subset when available
- local graph plus scoped remote graph
- optional gateway projection
- bounded dynamic behavior with profile-declared capacities

Allowed:

- richer diagnostics
- UDP/Ethernet transport
- limited management API

## Profile: linux-full

Required:

- full local graph
- remote runtime graph
- dynamic endpoint graph
- management API
- richer diagnostics
- native or ROS2 adapter backend support

## Profile: gateway-full

Required:

- downstream runtime representation
- upstream graph aggregation
- capability translation
- QoS downgrade visibility
- policy and diagnostics state
- stale representation handling through lease and boot identity

## Profile: sim-full

Required:

- full graph
- deterministic replay metadata
- fault-injection metadata
- simulated capability and health transitions

## Conformance Requirements

Each profile needs tests for:

- declared capability record is internally consistent
- unsupported features fail fast
- full graph cache is forbidden where declared
- no heap after init is enforced where declared
- max entity and payload limits are enforced
- downgrade is visible when accepted

