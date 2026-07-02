# Docker Environment

Docker support is planned but not implemented yet.

The intended direction is:

- macOS acts as editor/orchestration host.
- Ubuntu containers provide ROS2 and Linux validation.
- Native and ROS2 backend builds must use separate build/install/test-result directories.
- Future MCU cross-build images must be separate from ROS2 runtime images.
- Build products must be written under `GRCL_PLATFORM_ARTIFACT_ROOT`, not the repository root.
- Local default artifact root is the workspace-local directory `grcl-platform_ws/artifacts`.
