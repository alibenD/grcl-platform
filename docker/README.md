# Docker Environment

Docker support is planned but not implemented yet.

The intended direction is:

- macOS acts as editor/orchestration host.
- Ubuntu containers provide ROS2 and Linux validation.
- Native and ROS2 backend builds must use separate build/install/test-result directories.
- Future MCU cross-build images must be separate from ROS2 runtime images.

