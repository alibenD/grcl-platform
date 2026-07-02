# Development Environment

## Purpose

The development environment must let macOS host development while keeping Linux/ROS2 validation
reproducible and independent from local host differences.

## Host Roles

| Environment | Role |
|---|---|
| macOS | editor, git, documentation, orchestration |
| Ubuntu container | ROS2, native Linux, colcon, integration verification |
| MCU cross image | future FreeRTOS/NuttX/bare-metal cross builds |
| CI runner | reproducible checks and conformance matrix |

## Current Local Finding

The current macOS host has `colcon` available, but does not have `/opt/ros/humble/setup.bash` or a
Docker CLI in the checked environment. Therefore macOS must not be treated as the source of truth
for ROS2 runtime validation.

## Container Direction

Planned images:

- `ubuntu-ros-humble`
- `ubuntu-ros-jazzy`
- `mcu-cross`
- `docs-ci`

Detailed container boundaries are defined in
[Container Workspace Strategy](container-workspace-strategy.md).

Planned scripts:

- `scripts/dev-shell.sh`
- `scripts/test-docs.sh`
- `scripts/test-native.sh`
- `scripts/test-ros2.sh`
- `scripts/test-conformance.sh`

## Build Artifact Policy

`grcl-platform` is used through an explicit workspace layout:

```text
grcl-platform_ws/
  AGENTS.md
  artifacts/        # generated on demand; may be absent or deleted
  src/
    grcl-platform/
```

The workspace root is the preferred Codex session root because Docker containers can mount the whole
workspace and build scripts can access source and artifact paths without repeated permission
escalation. The repository remains the source tree only.

Build scripts must write generated products to the workspace-local `artifacts/` directory unless
`GRCL_PLATFORM_ARTIFACT_ROOT` overrides it. `artifacts/` is generated state, may be absent or
deleted, and must be created on demand by scripts before writing outputs.

Default local artifact root:

```text
/Users/aliben/Project/grcl-platform_ws/artifacts
```

Portable default expression for scripts:

```bash
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE_ROOT="$(cd "$REPO_ROOT/../.." && pwd)"
GRCL_PLATFORM_ARTIFACT_ROOT="${GRCL_PLATFORM_ARTIFACT_ROOT:-$WORKSPACE_ROOT/artifacts}"
mkdir -p "$GRCL_PLATFORM_ARTIFACT_ROOT"
```

Required artifact layout:

```text
$GRCL_PLATFORM_ARTIFACT_ROOT/
  colcon/
    native/
      build/
      install/
      log/
    ros2/
      build/
      install/
      log/
  cmake/
    grcl-c/
      debug/
      release/
  python/
    venv/
    wheels/
    cache/
  docs/
    link-check/
```

Colcon commands must use explicit bases:

```bash
colcon build \
  --build-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/build" \
  --install-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/install" \
  --log-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/log"
```

CMake commands must use explicit out-of-source build directories:

```bash
cmake -S "$REPO_ROOT/src/grcl-c" -B "$GRCL_PLATFORM_ARTIFACT_ROOT/cmake/grcl-c/debug"
```

CI must use a runner-local temp directory, for example:

```bash
GRCL_PLATFORM_ARTIFACT_ROOT="${RUNNER_TEMP:-/tmp}/grcl-platform-artifacts"
```

Repository-local `build/`, `install/`, and `log/` are ignored as a safety net, but their presence is
considered accidental local pollution rather than the intended workflow. The visible workspace
artifact path is preferred over hidden sibling directories so a fresh checkout makes the build
topology obvious.

## Verification Matrix

| Check | Environment |
|---|---|
| markdown/diff hygiene | macOS or CI |
| native Linux runtime | Ubuntu container |
| ROS2 adapter | Ubuntu ROS container |
| runtime capability schema tests | Ubuntu container |
| MCU profile static checks | MCU cross image |
| conformance tests | CI matrix |

Detailed conformance layers and CI stages are defined in
[Conformance And CI Strategy](conformance-and-ci-strategy.md).

## Rule

Do not claim ROS2 runtime verification from macOS unless the command actually runs in a configured
Ubuntu/ROS environment.

Do not claim a build script is acceptable unless it writes all build products under
`GRCL_PLATFORM_ARTIFACT_ROOT` or an explicitly provided equivalent out-of-source artifact root.
