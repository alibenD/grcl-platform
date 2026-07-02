# GRCL Platform Repository Rules

These rules are repository-local and define how agents should work in `grcl-platform`.

## Source Of Truth

- Do not rely on conversation history as the authoritative engineering record.
- New sessions should recover current state from `docs/status/current-context.md`.
- System-level decisions must be recorded in `docs/adr/`.
- Current architecture specifications must be recorded in `docs/architecture/`.
- Implementation sequencing must be recorded in `docs/plans/`.
- If a decision affects an implementation repository, update this repository first, then update the
  implementation repository.

## Session Bootstrap

- Prefer opening long-running Codex sessions at the workspace root:
  `/Users/aliben/Project/grcl-platform_ws`.
- The repository root is `/Users/aliben/Project/grcl-platform_ws/src/grcl-platform`.
- When running from the workspace root, use `git -C src/grcl-platform ...` for repository commands.
- Read the workspace-root `AGENTS.md`, this file, `README.md`, and
  `docs/status/current-context.md` before resuming architecture or implementation planning.
- The workspace-root session boundary is preferred because Docker containers, future multi-repo
  source trees, and generated artifacts need access to the whole workspace without repeated
  permission escalation.
- If prior chat history conflicts with repository documents, treat repository documents as the
  baseline and ask for confirmation before editing.

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

## Delegated Review And Completion Audit Rules

- A delegated subagent review only counts as evidence after it returns a completed result and the
  main agent inspects its findings or changes.
- A timed-out, shutdown, interrupted, or non-returning subagent produces no completion evidence.
- If delegated review was intended but does not complete, the main agent must run an explicit
  substitute completion audit before claiming the affected goal, plan, or milestone is complete.
- The substitute audit must derive requirements from the objective, user request, active plan, and
  repository rules, then map each requirement to current-state evidence.
- Documentation-only architecture work must at minimum run git status, diff hygiene, document
  inventory, internal link checks, and draft-marker scans before completion is claimed.
- Follow [Review And Verification Governance](docs/architecture/review-and-verification-governance.md)
  for delegated review states, substitute audit requirements, and final reporting rules.

## Build Artifact Rules

- Do not run builds that write `build/`, `install/`, `log/`, virtual environments, wheels, or cache
  directories into the repository root by default.
- The repository must normally live under a workspace layout:
  `grcl-platform_ws/src/grcl-platform`.
- Local scripts must use an out-of-source artifact root. The default local artifact root is the
  workspace-local directory `grcl-platform_ws/artifacts`.
- The artifact directory is generated state. It may be absent or deleted, and scripts must create it
  on demand before writing outputs.
- Scripts must allow `GRCL_PLATFORM_ARTIFACT_ROOT` to override the default artifact root.
- Colcon commands must pass explicit `--build-base`, `--install-base`, and `--log-base` paths under
  the artifact root.
- CMake commands must use explicit `-B` build directories under the artifact root.
- CI should use `$RUNNER_TEMP/grcl-platform-artifacts` or an equivalent runner-local temp
  directory.
