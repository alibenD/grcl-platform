# Repository Topology

## Decision

GRCL uses `grcl-platform` as the top-level modular monorepo during architecture stabilization.

The current `github.com/alibenD/grcl` repository remains an implementation repository and is
treated as a future `grcl-cpp` candidate.

## Intended Module Topology

```text
grcl-platform/
  src/
    grcl-c/
    grcl-cpp/
    grcl-py/
    grcl-idl/
    grcl-runtime-native/
    grcl-runtime-ros2/
    grcl-runtime-mcu/
    grcl-gateway/
    grcl-tools/
```

## Module Responsibilities

| Module | Responsibility | Initial Status |
|---|---|---|
| `grcl-c` | C ABI/core contract/runtime host/backend SPI | architecture only |
| `grcl-cpp` | idiomatic C++ SDK over `grcl-c` | current external prototype exists |
| `grcl-py` | Python SDK over `grcl-c` or private native shim | planned |
| `grcl-idl` | IDL grammar, codegen, type support, compatibility hashes | planned |
| `grcl-runtime-native` | native Linux runtime/backend | planned |
| `grcl-runtime-ros2` | ROS2 adapter/backend projection | planned |
| `grcl-runtime-mcu` | bare-metal, FreeRTOS, NuttX profiles and adapters | planned |
| `grcl-gateway` | MCU/gateway representation and graph aggregation | planned |
| `grcl-tools` | CLI, diagnostics, graph inspection, conformance tools | planned |

## Migration Policy

- Do not move `github.com/alibenD/grcl` into this repository until the topology plan is accepted.
- When migration starts, treat the current package as `grcl-cpp` candidate code.
- System-level ADRs live in `grcl-platform`.
- Module-specific implementation docs may live with modules once those modules exist.

## Diagram

See [repository-topology.mmd](../assets/repository-topology.mmd).

