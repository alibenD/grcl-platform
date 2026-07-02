# Repository Topology

## Decision

GRCL uses `grcl-platform` as the top-level modular monorepo during architecture stabilization.

The current `github.com/alibenD/grcl` repository remains an implementation repository and is
treated as a future `grcl-cpp` candidate.

## Local Workspace Topology

Local development should distinguish the repository root from the workspace root:

```text
grcl-platform_ws/
  AGENTS.md
  artifacts/        # generated on demand; may be absent or deleted
  src/
    grcl-platform/
```

The repository root is `grcl-platform_ws/src/grcl-platform`. The workspace root is
`grcl-platform_ws`. Build, test, coverage, generated files, logs, and other non-source artifacts
must be written to `grcl-platform_ws/artifacts` by default, or to
`GRCL_PLATFORM_ARTIFACT_ROOT` when that environment variable is explicitly set.

This mirrors the ROS-style workspace separation that the project expects for cross-repository
development, while avoiding in-repository build pollution.

Long-running Codex sessions should open at the workspace root rather than the repository root. This
keeps Docker workspace mounts, future sibling repositories, and generated artifacts inside one
sandbox boundary. Repository commands should still target `src/grcl-platform` explicitly.

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
