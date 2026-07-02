# Backend SPI Contract

## Purpose

The backend SPI defines what backend implementations must provide to the `grcl-c` core contract.
It does not define how each backend internally schedules work, stores graph state, or transports
data.

## Backend Families

| Backend | Role |
|---|---|
| native | Linux/native transport and local runtime behavior |
| ros2 | ROS2 adapter and GRCL graph projection over ROS2 behavior |
| mcu | profile-limited bare-metal/RTOS runtime |
| gateway | downstream runtime representation and capability translation |
| simulator | deterministic graph, runtime, and transport simulation |

## Required SPI Areas

| Area | Required Responsibilities |
|---|---|
| runtime | create, start, stop, destroy, identity, profile |
| graph | local graph, runtime participant state, graph projection support |
| capability | backend capability record, negotiation support |
| communication | endpoint create/destroy, channel open/close, publish/receive |
| scheduling | executor hooks or poll integration |
| memory | allocator/storage policy, bounded buffers |
| diagnostics | health, error reporting, degraded state |
| management | snapshot and event stream when profile supports it |

## Capability Query

Every backend must expose:

- supported runtime profile
- supported transports
- supported QoS subset
- supported graph projection modes
- memory and entity limits
- diagnostics level
- security capabilities

The core must reject backend registration when required profile capabilities are missing.

## ROS2 Adapter Rule

The ROS2 backend must adapt ROS2 behavior without exposing `rcl`, `rmw`, or `rclcpp` through public
GRCL APIs. It must also not assume that ROS2 directly provides GRCL's runtime participant graph.
The adapter is responsible for projecting the GRCL runtime view from ROS2 node graph, backend
metadata, and GRCL-owned state.

## MCU Backend Rule

The MCU backend must implement only the declared profile. Unsupported capabilities must fail fast
through capability negotiation or local object creation, not by partial runtime behavior.

## Baseline Decisions

- Backend implementations are internal to the `grcl-c` core contract boundary.
- Backend registration must expose runtime profile, capability record, graph projection support,
  memory/entity limits, diagnostics level, and security capabilities.
- ROS2 backend projection is owned by the adapter and cannot rely on ROS2/rmw exposing a GRCL
  runtime participant graph.
- MCU backends implement declared profiles rather than full GRCL by default.

## Remaining Contract Gates

- Exact C function-table shape requires the first `grcl-c` backend SPI header plan.
- Backend registration lifetime requires lifecycle tests.
- Error propagation from backend to core must preserve `grcl_result_t` categories and diagnostic
  detail separation.
- Management event schema requires a later management-plane ADR.
- Transport ownership and zero-copy handoff rules require native and MCU transport design input.
