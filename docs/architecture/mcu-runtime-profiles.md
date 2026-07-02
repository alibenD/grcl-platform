# MCU Runtime Profiles

## Purpose

MCU and RTOS targets do not implement full desktop GRCL by default. Each target implements a
declared profile with explicit capability, graph, memory, transport, and executor boundaries.

## Profiles

| Profile | Target | Graph Obligation |
|---|---|---|
| `baremetal-min` | bare-metal MCU | local graph + static manifest + peer/gateway runtime summary |
| `rtos-basic` | FreeRTOS-like systems | local graph + session peer summary + optional interest-scoped deltas |
| `rtos-posix-lite` | NuttX/Zephyr-like POSIX subset | local graph + scoped remote graph + optional gateway projection |
| `linux-full` | Linux/Yocto/Ubuntu | full local graph + remote runtime graph |
| `gateway-full` | gateway process/device | represented MCU graph + upstream graph aggregation |
| `sim-full` | simulator/runtime test harness | full graph + deterministic replay/fault-injection metadata |

## Required Profile Fields

Each profile must declare:

- minimum capability exchange support
- maximum required remote graph depth
- dynamic endpoint delta support
- static manifest requirement
- gateway representation support
- whether full graph cache is forbidden
- allocation policy
- max entities
- max payload
- transport set
- executor mode
- diagnostics level

## Initial OS Adapter Targets

- bare metal
- FreeRTOS
- NuttX
- Linux

## Baseline Decisions

- The first profile descriptor schema is defined in [MCU Profile Schema](mcu-profile-schema.md).
- `baremetal-min` and constrained RTOS profiles must not be required to maintain a full distributed
  graph cache.
- Unsupported capabilities fail fast through local creation checks or capability negotiation.
- Static manifest and gateway representation are first-class graph projection modes.
- FreeRTOS-like and NuttX/Zephyr-like adapters are planned as separate backend/profile adapters.

## Remaining Design Gates

- Static manifest file format requires a schema artifact before implementation claims support.
- RTOS adapter and transport adapter function-table details require a backend SPI implementation
  plan.
- Benchmark acceptance criteria require measured targets from at least one MCU or simulator profile
  and must not be invented from documentation alone.
