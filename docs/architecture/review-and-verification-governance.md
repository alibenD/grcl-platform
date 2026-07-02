# Review And Verification Governance

## Purpose

This document defines how GRCL Platform uses delegated review, local audit, and verification
evidence. It exists to prevent a timed-out or skipped review from being treated as proof that a
goal is complete.

For implementation-stage work, this document is used together with
[Agentic Delivery Governance](agentic-delivery-governance.md). The delivery governance defines the
main-agent, implementation-subagent, and independent-audit-subagent workflow. This document defines
how review evidence is evaluated and reported.

## Core Rule

A delegated subagent review is useful evidence only when it returns a completed result that is
inspected and integrated. A timed-out, closed, interrupted, or non-returning subagent produces no
completion evidence.

If delegated review does not complete, the main agent must perform an explicit substitute
completion audit before claiming that the affected goal, plan, or milestone is complete.

For implementation tasks, substitute audit is not a normal replacement for the required independent
audit subagent. If the audit subagent fails to return, the main agent may run a local substitute
audit only to determine whether the task is blocked or safe to re-dispatch. The main agent must not
advance to the next implementation task unless an independent audit has completed, or the user
explicitly approves bypassing the gate for that task.

## Delegated Review States

| State | Evidence Value | Required Action |
|---|---|---|
| `completed` with final answer | usable after inspection | review findings, integrate or reject with reasons |
| `completed` with file changes | usable after diff and verification | inspect diff, run relevant gates |
| `timeout` | no evidence | run substitute completion audit or leave work incomplete |
| `shutdown` before result | no evidence | run substitute completion audit or leave work incomplete |
| `failed` | negative evidence | inspect failure and decide whether blocked or locally auditable |

## Substitute Completion Audit

When delegated review is unavailable, the main agent must run a substitute audit that covers the
same requirement scope. The audit must include:

1. Derive concrete requirements from the current objective, user request, active plan, and relevant
   repository rules.
2. Map each requirement to authoritative evidence, such as files, command output, rendered docs,
   tests, schemas, or git state.
3. Inspect the current files directly rather than relying on conversation memory.
4. Run verification commands that match the requirement scope.
5. Record which delegated review did not complete and which substitute audit evidence was used.
6. Do not mark the goal complete if any requirement remains unverified.

## Minimum Documentation Audit Gate

For documentation-only architecture work, the substitute audit must include at least:

```bash
git status --short --branch
git diff --check HEAD
find docs -name '*.md' -print | sort
```

It must also check internal Markdown links and unfinished-content markers. Until a repository
script exists, the link check may be run through an inline Python command, but the evidence must be
reported.

Required unfinished-content scan:

```bash
rg -n "T[B]D|T[O]DO|fill[ ]in|implement[ ]later|place[ ]holder" README.md AGENTS.md docs docker scripts src tests manifests
```

The scan is expected to return no matches. If it returns matches inside intentionally documented
commands, the command itself must use non-self-matching patterns as shown above.

## Completion Gate

Before a goal or plan can be called complete:

- all explicit deliverables must exist.
- all relevant navigation links must resolve.
- git state must show no unintended uncommitted changes.
- documentation hygiene checks must pass.
- any delegated review dependency must either have completed and been inspected, or must be
  replaced by a documented substitute audit.
- implementation tasks must have a completed independent audit report unless the user explicitly
  approved a bypass.

If these conditions are not met, the work is incomplete even if most files look reasonable.

## Reporting Rule

Final reports must distinguish:

- delegated review completed and findings were integrated.
- delegated review did not complete and substitute audit was performed.
- delegated review did not complete and the goal remains incomplete or blocked.

The phrase "subagent timed out, but it did not block" is insufficient by itself. It must be paired
with the substitute audit evidence that proves no required coverage was lost.
