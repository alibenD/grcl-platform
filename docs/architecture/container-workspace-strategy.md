# Container Workspace Strategy

## Purpose

GRCL Platform uses macOS as the editing and orchestration host, but Linux containers are the
validation boundary for native runtime, ROS2 adapter, and future MCU cross-build work. The container
strategy must preserve a clean source tree while making workspace-level artifacts visible and
repeatable.

## Workspace Mount Model

The host workspace root is:

```text
/Users/aliben/Project/grcl-platform_ws
```

The container workspace root should be:

```text
/workspace/grcl-platform_ws
```

Expected container layout:

```text
/workspace/grcl-platform_ws/
  src/
    grcl-platform/
  artifacts/        # generated on demand
```

Containers should mount the whole workspace root, not only `src/grcl-platform`, so source,
artifacts, future sibling repositories, and cross-module build outputs share one sandbox boundary.

## Standard Environment Variables

Inside containers:

```bash
GRCL_PLATFORM_WORKSPACE_ROOT=/workspace/grcl-platform_ws
GRCL_PLATFORM_REPO_ROOT=/workspace/grcl-platform_ws/src/grcl-platform
GRCL_PLATFORM_ARTIFACT_ROOT=/workspace/grcl-platform_ws/artifacts
```

Scripts must allow `GRCL_PLATFORM_ARTIFACT_ROOT` to override the default. Scripts must create the
artifact root on demand before writing outputs.

## Image Boundaries

| Image | Role | Must Include | Must Not Include |
|---|---|---|---|
| `docs-ci` | documentation checks | Python 3, ripgrep, git | ROS2 runtime assumptions |
| `ubuntu-native` | native Linux build and tests | C/C++ toolchain, CMake, colcon if needed | ROS2-only dependencies as hard requirements |
| `ubuntu-ros-humble` | ROS2 Humble adapter validation | ROS2 Humble, colcon, native toolchain | MCU cross toolchains as defaults |
| `ubuntu-ros-jazzy` | ROS2 Jazzy compatibility validation | ROS2 Jazzy, colcon, native toolchain | MCU cross toolchains as defaults |
| `mcu-cross` | MCU profile and future cross-build validation | selected cross toolchains, static analyzers | ROS2 desktop runtime as a dependency |

The first implementation phase may document image directories without producing final Dockerfiles.
Dockerfiles require a separate accepted implementation task.

## Artifact Layout

Container scripts must use the same artifact layout as local scripts:

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
  docs/
    link-check/
  test-results/
    native/
    ros2/
    conformance/
```

Colcon commands must pass explicit bases. CMake commands must pass explicit `-B` paths. No command
may rely on repository-root `build/`, `install/`, or `log/`.

## Permission Model

Local Docker commands should run with host UID/GID mapping where possible so generated files under
`artifacts/` are removable by the host user. If a container must run as root, generated outputs must
still stay under `artifacts/` so cleanup is isolated.

The repository must not require `sudo` cleanup for normal generated files.

## Initial Script Responsibilities

Future scripts should follow this split:

| Script | Responsibility |
|---|---|
| `scripts/env.sh` | resolve workspace, repo, and artifact roots; create artifact root on demand |
| `scripts/dev-shell.sh` | open an interactive container with workspace mounted |
| `scripts/test-docs.sh` | run documentation-only checks without requiring ROS2 |
| `scripts/test-native.sh` | run native Linux checks inside Ubuntu native container |
| `scripts/test-ros2.sh` | run ROS2 adapter checks inside a selected ROS2 container |
| `scripts/test-conformance.sh` | run schema, profile, SDK, and backend conformance checks |

Scripts must fail clearly when Docker is unavailable. They must not silently fall back to macOS and
claim Linux or ROS2 validation.

## Local Host Reality

The current macOS environment may not have Docker CLI or ROS2 installed. That is acceptable for
architecture work. It means:

- macOS can run documentation checks.
- macOS cannot be used as proof of ROS2 runtime validation.
- Docker implementation work must include explicit unavailable-tool diagnostics.
- CI or a configured Ubuntu host becomes the source of truth for Linux and ROS2 runtime gates.

## Relationship To Agentic Delivery

Long-running Codex sessions should open at the workspace root so subagents and scripts can access
the same workspace paths. Implementation subagents must receive the workspace root and repository
root in their task briefs. Audit subagents must check that generated files stay out of
`src/grcl-platform`.
