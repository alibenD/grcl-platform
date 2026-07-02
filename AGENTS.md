# GRCL Platform Repository Rules

These rules are repository-local and define how agents should work in `grcl-platform`.

## Source Of Truth

- Do not rely on conversation history as the authoritative engineering record.
- System-level decisions must be recorded in `docs/adr/`.
- Current architecture specifications must be recorded in `docs/architecture/`.
- Implementation sequencing must be recorded in `docs/plans/`.
- If a decision affects an implementation repository, update this repository first, then update the
  implementation repository.

## Current Development Mode

- GRCL implementation work is paused until the platform architecture plan is reviewed.
- Do not create runtime code, SDK code, or build-system code unless the task plan explicitly moves
  that work into an accepted implementation phase.
- Documentation, architecture diagrams, dependency analysis, and task planning are allowed.

## Architecture Rules

- `grcl-c` is the core contract boundary for runtime lifecycle, object ownership, backend SPI,
  capability exchange, controlled storage, and cross-language semantic consistency.
- `grcl-cpp`, `grcl-py`, and future SDKs must not define independent runtime semantics.
- MCU/RTOS runtimes implement declared profiles, not full GRCL by default.
- Runtime capability exchange and scoped graph behavior must follow the platform architecture docs.
- ROS2 compatibility must be described accurately: ROS2 exposes a node graph to users; GRCL's
  runtime participant graph is a GRCL platform abstraction.

## Planning Rules

- Every non-trivial implementation effort must have a task plan in `docs/plans/`.
- A task plan must include dependencies, sequencing, validation, and explicit user review points.
- Do not mark a task completed without fresh verification evidence.
- If verification cannot run locally, record the exact environmental blocker.

