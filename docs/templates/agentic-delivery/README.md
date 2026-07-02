# Agentic Delivery Templates

These templates define the file-based contract between the main agent, implementation subagents,
and independent audit subagents.

Use them under a workspace-local run directory:

```text
<workspace-root>/.local/agentic-runs/<plan-id>/<task-id>/
```

The run directory is generated state and is not committed. Durable results must be summarized in
the active plan, status document, and commit history.

## Templates

- [Task Brief](task-brief.md)
- [Implementation Report](implementation-report.md)
- [Audit Report](audit-report.md)
- [Ledger](ledger.md)

## Required Flow

1. Main agent creates `task-brief.md` and `ledger.md`.
2. Implementation subagent reads `task-brief.md`.
3. Implementation subagent writes `implementation-report.md`.
4. Main agent inspects implementation report and diff.
5. Independent audit subagent reads `task-brief.md` and `implementation-report.md`.
6. Audit subagent writes `audit-report.md`.
7. Main agent either marks task accepted or creates a narrowed fix task.
8. The next planned task starts only after audit acceptance.
