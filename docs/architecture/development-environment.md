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

Planned scripts:

- `scripts/dev-shell.sh`
- `scripts/test-docs.sh`
- `scripts/test-native.sh`
- `scripts/test-ros2.sh`
- `scripts/test-conformance.sh`

## Verification Matrix

| Check | Environment |
|---|---|
| markdown/diff hygiene | macOS or CI |
| native Linux runtime | Ubuntu container |
| ROS2 adapter | Ubuntu ROS container |
| runtime capability schema tests | Ubuntu container |
| MCU profile static checks | MCU cross image |
| conformance tests | CI matrix |

## Rule

Do not claim ROS2 runtime verification from macOS unless the command actually runs in a configured
Ubuntu/ROS environment.

