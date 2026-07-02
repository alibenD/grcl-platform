# SDK Platform Governance

## Purpose

If GRCL is opened to third-party developers, it becomes a platform interface rather than only a
middleware library. SDK governance must therefore be designed before exposing control, voice,
whole-machine, or hardware-facing extension capabilities.

## Required Governance Areas

- SDK manifest
- capability and permission model
- resource budget model
- observability and audit events
- conformance test kit
- signing and provenance
- private registry before public marketplace
- quality levels for SDK risk classes

## Suggested SDK Quality Levels

| Level | Meaning |
|---|---|
| `experimental` | internal experiments and prototypes |
| `tooling` | diagnostics and non-control tooling |
| `observability` | read-only graph, metrics, logs, tracing |
| `planning-candidate` | planning logic requiring simulation validation |
| `control-candidate` | lab-only control logic requiring safety review |
| `production-control` | signed, audited, conformance-passing control extension |

## Baseline Decisions

- SDK extensions must declare manifests before being considered stable.
- SDKs that touch control, planning, voice, or hardware-facing behavior require explicit capability
  and permission policy.
- Stable SDK claims require conformance tests, provenance, and audit evidence.
- A private registry or internal catalog should precede any public marketplace.
- SDK quality levels are risk classes, not marketing labels.

## Remaining Design Gates

- Manifest schema requires a dedicated schema artifact.
- Permission policy language requires a later ADR because it affects runtime enforcement and SDK
  compatibility.
- Runtime enforcement points must be mapped to `grcl-c`, backend SPI, and management APIs.
- Registry metadata, signing, and provenance require release-governance planning.
- Simulation and fault-injection gates require simulator profile and conformance suite artifacts.
