# Agentic Delivery Governance

## Purpose

GRCL Platform uses agentic execution for implementation work, but architecture decisions and task
planning require stronger reasoning than mechanical implementation. This document defines the
repeatable delivery model, model policy, subagent roles, file-based exchange contract, review loop,
and completion gate.

Use [Task Workflow Governance](task-workflow-governance.md) first to classify the task, determine
whether fast-path handling is forbidden, and confirm whether this document applies. This document is
the implementation-task specialization of the broader repository workflow.

The goal is to prevent implementation drift in a heterogeneous middleware project where C, C++,
Python, ROS2, Docker, MCU profiles, wire protocol, and SDK governance must stay consistent with the
architecture documents.

## Scope

This governance applies to:

- architecture design and ADR authoring.
- implementation task planning.
- code implementation after a plan is accepted.
- documentation changes that define system behavior.
- independent audit of implementation against architecture and plans.

It does not replace normal Git, CI, tests, or human review. It defines how the main agent and
subagents exchange information and when work may proceed.

It also does not replace task classification, TDD policy, commit-boundary policy, or durable-state
update duties. Those rules are owned by [Task Workflow Governance](task-workflow-governance.md).

## Model Policy

| Work Type | Default Model Policy | Rationale |
|---|---|---|
| Architecture design | GPT-5.5, high effort | Requires cross-system reasoning and long-horizon trade-off analysis. |
| ADRs and platform specs | GPT-5.5, high effort | Decisions become durable process assets and affect multiple repositories. |
| Task planning | GPT-5.5, high effort | Plans must be complete enough for isolated implementers with limited context. |
| Implementation subagent | GPT-5.4, medium effort by default | Most accepted tasks should be narrow and mechanically executable. |
| Complex implementation subagent | Adaptive upgrade to GPT-5.5 or high effort | Required when the task crosses module boundaries, semantics, concurrency, ABI, protocol, or safety constraints. |
| Independent audit subagent | GPT-5.5, high effort by default | Audit requires comparing implementation, plan, architecture, tests, and negative cases. |
| Main coordinating agent | Use the strongest available planning model | The main agent owns sequencing, context curation, and gate decisions. |

The main agent may downgrade or upgrade implementation models by task complexity, but must record
the chosen model policy in the task brief.

## Roles

### Main Agent

The main agent owns:

- classifying the task before implementation dispatch.
- reading the active plan and architecture context.
- decomposing work into isolated tasks.
- creating file-based work packages.
- dispatching implementation subagents.
- dispatching independent audit subagents.
- deciding whether audit feedback is valid.
- creating follow-up fix tasks when audit fails.
- updating task status, ledger, and final reports.

The main agent must not treat a subagent timeout, partial answer, or uninspected result as evidence.

### Implementation Subagent

An implementation subagent owns exactly one implementation task or one explicit fix task. It must:

- read the task brief provided by the main agent.
- read only the referenced architecture, plan, and source files unless it needs more context.
- follow the repository TDD rule when the task changes code or verifiable behavior.
- make the requested changes.
- run the required verification commands or record exact blockers.
- write an implementation report.
- return one status: `done`, `done_with_concerns`, `needs_context`, or `blocked`.

Implementation subagents must not silently broaden scope or reinterpret architecture decisions.

### Independent Audit Subagent

An audit subagent must be independent from the implementation subagent. It must:

- read the task brief.
- read the implementation report.
- inspect the actual diff and current files.
- compare implementation against the plan and architecture documents.
- run or review verification evidence.
- write an audit report.
- return one status: `accepted`, `accepted_with_notes`, `rejected`, or `blocked`.

The audit subagent must not modify implementation files. If it finds issues, it reports them with
evidence and recommended fix scope. The main agent dispatches a new fix task.

## Execution State Machine

This state machine starts after the broader workflow has already reached `implement` for an
implementation-classified task.

```text
planned
  -> brief_created
  -> implementation_dispatched
  -> implementation_done
  -> audit_dispatched
  -> audit_accepted
  -> task_complete
```

Failure and loop states:

```text
implementation_done
  -> audit_rejected
  -> fix_brief_created
  -> fix_dispatched
  -> fix_done
  -> audit_dispatched
```

Blocking states:

```text
implementation_dispatched -> needs_context
implementation_dispatched -> blocked
audit_dispatched -> audit_blocked
```

The main agent may proceed to the next task only after the current task reaches
`audit_accepted`.

## File-Based Exchange Contract

Subagent exchange must use files, not only chat messages. The default working directory for task
exchange is:

```text
<workspace-root>/.local/agentic-runs/<plan-id>/<task-id>/
```

Example:

```text
grcl-platform_ws/
  .local/
    agentic-runs/
      2026-07-02-grcl-platform-module-skeleton-plan/
        task-001-docs-test-script/
          task-brief.md
          implementation-report.md
          audit-report.md
          ledger.md
```

`.local/` is generated workspace state and should not be committed. Durable summaries must be
promoted into the active plan, status document, or commit messages.

Required exchange files:

| File | Writer | Reader | Purpose |
|---|---|---|---|
| `task-brief.md` | main agent | implementation subagent, audit subagent | Complete task context, constraints, acceptance criteria, and verification commands. |
| `implementation-report.md` | implementation subagent | main agent, audit subagent | What changed, evidence, concerns, blockers, and commit/diff references. |
| `audit-report.md` | audit subagent | main agent | Architecture/plan/code/test cross-check and pass/fail result. |
| `ledger.md` | main agent | main agent, future sessions | Per-task status transitions and final decision trail. |

Optional files:

| File | Writer | Purpose |
|---|---|---|
| `questions.md` | subagent | Context questions before work can continue. |
| `fix-brief.md` | main agent | Narrow follow-up task generated from audit findings. |
| `verification-output.txt` | subagent | Command output excerpts when useful and not too large. |

## Task Brief Requirements

Every task brief must include:

- task ID and title.
- plan path and task section.
- base commit or working tree baseline.
- selected model policy.
- role: implementation or fix.
- repository path and workspace path.
- required architecture references.
- required source files to inspect.
- exact files allowed to modify.
- explicit non-goals.
- acceptance criteria.
- required verification commands.
- expected output files.
- status vocabulary.
- escalation rules.

The brief must be self-contained enough that a subagent does not need previous chat history.

## Implementation Report Requirements

Every implementation report must include:

- final status.
- task ID.
- files changed.
- commits created or diff range.
- tests and checks run.
- exact blockers if checks could not run.
- assumptions made.
- deviations from task brief.
- remaining concerns.
- audit readiness statement.

`done_with_concerns` can proceed to audit only after the main agent decides the concerns are
acceptable for audit. `needs_context` and `blocked` cannot proceed to audit.

## Audit Report Requirements

Every audit report must include:

- final status.
- task ID.
- implementation report reviewed.
- architecture documents checked.
- plan requirements checked.
- changed files inspected.
- verification evidence reviewed or rerun.
- findings by severity.
- explicit pass/fail decision for each acceptance criterion.
- required fix tasks if rejected.

`accepted_with_notes` may proceed only when notes are non-blocking and explicitly recorded by the
main agent.

## Review Loop Rules

- Implementation and audit must be performed by different subagents.
- The audit subagent must not be the same execution context as the implementer.
- A failed audit creates a new fix task; the main agent must not patch ad hoc unless the fix is a
  trivial coordination correction and is documented.
- The fix task must receive the original brief, implementation report, audit report, and narrowed
  fix scope.
- After the fix, a new independent audit is required.
- The main agent cannot move to the next planned task until audit passes.
- If the same task fails audit repeatedly because the plan is ambiguous or wrong, pause and revise
  the plan before continuing.
- The task remains incomplete until the verified result is committed under the repository
  `one task -> one commit` policy and the relevant durable state is updated.

## Architecture Cross-Check Gate

For GRCL implementation tasks, audit must compare the implementation against relevant architecture
documents, not only tests. At minimum, audit should answer:

- Does the implementation match the accepted task plan?
- Does it preserve `grcl-c` as the core contract boundary?
- Does it avoid creating independent SDK semantics in wrappers?
- Does it respect MCU profile and no-full-graph assumptions where relevant?
- Does it preserve workspace artifact isolation?
- Does it avoid claiming ROS2/rmw capabilities that the architecture forbids?
- Are tests and docs aligned with the behavior introduced?

The exact checklist must be task-specific and listed in the task brief.

## Main Agent Queue Rules

The main agent maintains a queue with one active implementation task at a time unless the tasks
touch disjoint files and the plan explicitly allows parallelism. By default:

- do not dispatch multiple implementation subagents against the same repo simultaneously.
- do not start the next task while audit is pending.
- do not collapse implementation and audit into one subagent.
- update the ledger after every state transition.
- summarize only inspected evidence in final reports.

## Reusable Pattern

This governance is intended to become a reusable pattern for other repositories:

1. Architecture and planning use a high-reasoning model.
2. Each implementation task receives a self-contained file brief.
3. Each implementation subagent writes a file report.
4. A separate audit subagent compares architecture, plan, implementation, and verification.
5. Rejected audits generate narrow fix tasks.
6. The loop repeats until independent audit passes.
7. The main agent records durable summary state before moving forward.

Templates live in [Agentic Delivery Templates](../templates/agentic-delivery/README.md).
