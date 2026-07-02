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

## Open Design Items

- Exact profile schema.
- Static manifest format.
- RTOS adapter contract.
- Transport adapter contract.
- Benchmark acceptance criteria.

