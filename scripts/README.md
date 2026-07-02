# Scripts

This directory will hold development and verification scripts after the development environment
design is accepted.

Planned scripts:

- `env.sh`
- `dev-shell.sh`
- `test-docs.sh`
- `test-native.sh`
- `test-ros2.sh`
- `test-conformance.sh`

All build/test scripts must source `scripts/env.sh` and write generated artifacts under
`GRCL_PLATFORM_ARTIFACT_ROOT`. They must not default to repository-root `build/`, `install/`, or
`log/` directories.
