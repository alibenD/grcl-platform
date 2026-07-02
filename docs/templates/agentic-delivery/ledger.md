# Agentic Task Ledger

## Identity

- Plan ID:
- Task ID:
- Task title:
- Main agent:
- Workspace root:
- Repository root:

## State Transitions

| Time | State | Actor | Evidence |
|---|---|---|---|
| timestamp | planned | main | plan path |

Allowed states:

- `planned`
- `brief_created`
- `implementation_dispatched`
- `needs_context`
- `blocked`
- `implementation_done`
- `audit_dispatched`
- `audit_accepted`
- `audit_rejected`
- `fix_brief_created`
- `fix_dispatched`
- `fix_done`
- `task_complete`

## Artifacts

- Task brief:
- Implementation report:
- Audit report:
- Fix brief:

## Commits

- commit:

## Main Agent Decisions

Record every gate decision:

- decision:
- evidence:
- reason:

## Final Outcome

Choose exactly one:

- `complete`
- `blocked`
- `deferred`

Outcome:

Reason:
