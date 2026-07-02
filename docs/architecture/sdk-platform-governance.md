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

## Open Design Items

- Manifest schema.
- Permission policy language.
- Runtime enforcement points.
- Registry metadata.
- Simulation and fault-injection gates.

