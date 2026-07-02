# GRCL Platform System Overview

## Purpose

GRCL is a heterogeneous robot middleware platform. It must support Linux hosts, Jetson-class
edge compute, ROS2 interop, native runtimes, gateway nodes, MCU/RTOS runtimes, simulators, tools,
and future third-party SDKs while preserving one coherent programming and runtime model.

## Top-Level Architecture

```text
Applications / Robot Features / Third-Party SDKs
  -> Language SDKs: grcl-cpp, grcl-py, grcl-rs, grcl-c
    -> GRCL-C Core Contract
      -> Backend SPI and Runtime Host Contracts
        -> Native Runtime
        -> ROS2 Adapter Runtime
        -> MCU/RTOS Runtime Profiles
        -> Gateway Runtime
        -> Simulator Runtime
```

## System Goals

- Keep core runtime semantics consistent across MCU, Linux, ROS2, native, gateway, and simulator
  deployments.
- Allow MCU/RTOS runtimes to implement profile-based subsets rather than full desktop runtime
  behavior.
- Make runtime capability exchange explicit before cross-runtime channels are opened.
- Keep language SDKs convenient without allowing them to define conflicting core semantics.
- Support future SDK governance, permissions, conformance tests, observability, and registry
  workflows.

## Non-Goals For The Current Phase

- Do not implement runtime code in this repository yet.
- Do not migrate the current `github.com/alibenD/grcl` C++ package until repository topology is
  reviewed.
- Do not create a public SDK marketplace before manifest, permissions, conformance, and signing
  rules exist.

## Diagram

See [system-overview.mmd](../assets/system-overview.mmd).

