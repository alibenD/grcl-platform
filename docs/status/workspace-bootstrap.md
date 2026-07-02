# Workspace Bootstrap

## Purpose

GRCL Platform uses the workspace root as the recommended Codex session directory:

```text
/Users/aliben/Project/grcl-platform_ws
```

The Git repository is a child directory:

```text
/Users/aliben/Project/grcl-platform_ws/src/grcl-platform
```

This layout lets long-running tasks use one sandbox boundary for source repositories, generated
artifacts, Docker mounts, and future multi-repository orchestration.

## Workspace Root AGENTS.md

The workspace root should contain a host-local `AGENTS.md` file that points Codex to the repository
and current context file:

```text
grcl-platform_ws/
  AGENTS.md
  src/
    grcl-platform/
```

The workspace-root `AGENTS.md` is not tracked by the `grcl-platform` Git repository because it lives
above the repository root. Its durable source of truth is this repository's bootstrap guidance and
the current context file.

If the workspace-root `AGENTS.md` is missing, recreate it with these minimum instructions:

```markdown
# GRCL Platform Workspace Rules

This is the workspace root for GRCL Platform. It is intentionally not the Git repository root.

The primary repository is `src/grcl-platform`.

When starting a Codex session in this workspace:

1. Run `git -C src/grcl-platform status --short --branch`.
2. Read `src/grcl-platform/AGENTS.md`.
3. Read `src/grcl-platform/README.md`.
4. Read `src/grcl-platform/docs/status/current-context.md`.
5. Read the active plan referenced by the current-context file before editing.

Architecture design, ADR writing, and task planning should use GPT-5.5 high effort when available.
Implementation-stage work must follow
`src/grcl-platform/docs/architecture/agentic-delivery-governance.md`.

The main agent manages the task queue. Implementation subagents receive file-based task briefs under
`.local/agentic-runs/<plan-id>/<task-id>/`. After each implementation subagent completes, a separate
independent audit subagent must compare the implementation against the task plan, architecture
documents, changed files, and verification evidence before the main agent proceeds.

Generated artifacts belong under `artifacts/`. The directory is generated on demand and may be
absent or deleted.
```

## Bootstrap Check

From the workspace root:

```bash
test -f AGENTS.md
test -d src/grcl-platform/.git
git -C src/grcl-platform status --short --branch
sed -n '1,220p' src/grcl-platform/docs/status/current-context.md
```

If these checks fail, do not continue architecture or implementation work until the workspace
layout is repaired or the user confirms an alternate layout.
