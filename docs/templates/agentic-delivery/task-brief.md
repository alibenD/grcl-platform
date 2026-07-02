# Task Brief

## Identity

- Plan ID:
- Task ID:
- Task title:
- Role: `implementation` or `fix`
- Main agent:
- Assigned subagent:
- Model policy:
- Base commit:
- Workspace root:
- Repository root:

## Source Plan

- Plan path:
- Plan section:
- Related issue or decision:

## Required Context

Read these files before editing:

- `AGENTS.md`
- `README.md`
- `docs/status/current-context.md`

Task-specific architecture references:

- path:
- path:

Task-specific source references:

- path:
- path:

## Objective

Describe the exact outcome expected from this task.

## Allowed Changes

The subagent may modify only these files or directories:

- path:
- path:

## Non-Goals

The subagent must not:

- change unrelated architecture decisions.
- broaden the task scope.
- edit files outside the allowed change list.
- treat previous chat history as source of truth.

## Acceptance Criteria

- criterion:
- criterion:

## Architecture Cross-Check

The implementation must remain consistent with:

- `grcl-c` as core contract boundary when relevant.
- SDK wrappers not defining independent runtime semantics when relevant.
- MCU profile constraints when relevant.
- workspace artifact isolation when relevant.
- corrected ROS2 graph baseline when relevant.

## Verification Commands

Run from the workspace root unless specified:

```bash
git -C src/grcl-platform status --short --branch
```

Additional task-specific commands:

```bash
command
```

## Required Output Files

Write the implementation result to:

```text
implementation-report.md
```

If blocked before implementation, write:

```text
questions.md
```

or return `blocked` in `implementation-report.md` with exact evidence.

## Status Vocabulary

The implementation subagent must return exactly one status:

- `done`
- `done_with_concerns`
- `needs_context`
- `blocked`

## Escalation Rules

Return `needs_context` if the task lacks required facts.

Return `blocked` if commands cannot run, permissions prevent progress, or the task conflicts with
architecture documents.

Do not invent missing decisions.
