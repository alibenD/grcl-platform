# Task Workflow Governance

## Purpose

This document defines the repository-local task taxonomy, lifecycle states, fast-path limits, TDD
requirements, commit boundaries, and durable-state update rules for GRCL Platform.

It sits above implementation-specific agent dispatch rules. Use this document to decide what kind
of task is being executed, which gates apply, when fast-path handling is allowed, and what counts
as completion. Use [Agentic Delivery Governance](agentic-delivery-governance.md) to execute
implementation tasks once this workflow has classified them.

## Scope

This governance applies to all non-trivial repository work in `grcl-platform`, including:

- architecture design and architecture iteration
- runtime, SDK, schema, tooling, and test implementation
- documentation-only behavior changes
- release, queue, current-context, template, and governance maintenance

Pure exploration that does not modify repository state may stop before implementation and commit,
but it must not be reported as a completed task.

## Task Types

Every task must be classified before work begins. The main agent owns classification and must record
it in the plan, task brief, or durable status update when the task is material.

| Task type | Description | Default gate strength |
|---|---|---|
| `architecture design` | New architecture direction, subsystem boundary, ADR, or cross-module contract design | proposal, review gate, verification, commit, durable update |
| `architecture iteration` | Clarify, narrow, revise, or reconcile accepted architecture docs without changing code | analyze/plan, proposal, review gate, verification, commit, durable update |
| `feature dev` | Add new behavior, API, schema, tool, fixture, or implementation capability | full workflow, TDD, implementation subagent, independent audit |
| `bug fix` | Correct behavior that is wrong, incomplete, or inconsistent with docs or tests | full workflow, reproduction first, TDD, implementation subagent, independent audit |
| `refactor` | Structural cleanup that claims no external behavior change | full workflow, behavior-protection tests, independent audit |
| `test/infrastructure` | Conformance harnesses, validation scripts, fixture checkers, artifact-path enforcement, CI-facing local scaffolding | full workflow, TDD when behavior changes, independent audit |
| `docs-only behavior change` | Documentation-only change that defines system behavior, architecture meaning, delivery policy, or operator expectations | proposal, review gate, documentation audit, commit, durable update |
| `docs/editorial` | Spelling, dead links, formatting, or wording cleanup with no behavioral or governance meaning change | fast path allowed, verify, commit, durable note |
| `release/governance` | Queue state, current-context, templates, workflow rules, quality gates, or release-process updates | proposal, review gate, documentation audit, commit, durable update |

If a task touches multiple categories, classify it by the highest-risk behavior. A task that mixes
`docs/editorial` with any behavior, contract, or governance change is not editorial.

## Canonical Workflow

The default task state machine is:

```text
intake
  -> classify
  -> analyze/plan
  -> proposal
  -> review gate
  -> implement
  -> audit/verify
  -> commit
  -> durable state update
  -> close
```

State meanings:

- `intake`: capture the request and confirm the repository boundary.
- `classify`: assign one task type and decide whether implementation subagents are required.
- `analyze/plan`: inspect current docs, plans, code, tests, and queue state before changing files.
- `proposal`: state the intended approach, scope, and acceptance boundary.
- `review gate`: obtain the required approval or accepted baseline before implementation proceeds.
- `implement`: make the scoped change only.
- `audit/verify`: run the task-specific checks and any required independent audit.
- `commit`: create the task-bounded commit after verification passes.
- `durable state update`: update recovery docs, queue state, ledgers, or indexes as needed so the
  task can be recovered without chat history.
- `close`: report completion using only inspected evidence.

## Gate Matrix

| Task type | Proposal required | Review gate required | Implementation subagent required | Independent audit required | Commit required | Durable update required |
|---|---|---|---|---|---|---|
| `architecture design` | yes | yes | no by default | no by default | yes | yes |
| `architecture iteration` | yes | yes | no by default | no by default | yes | yes |
| `feature dev` | yes | yes via accepted plan/brief | yes | yes | yes | yes |
| `bug fix` | yes | yes via accepted plan/brief | yes | yes | yes | yes |
| `refactor` | yes | yes via accepted plan/brief | yes | yes | yes | yes |
| `test/infrastructure` | yes | yes via accepted plan/brief | yes for implementation work | yes | yes | yes |
| `docs-only behavior change` | yes | yes | no by default | no by default | yes | yes |
| `docs/editorial` | minimal | no separate review gate | no | no | yes | yes, but note-level is sufficient |
| `release/governance` | yes | yes | no by default | no by default | yes | yes |

Implementation tasks may not skip `commit` or `durable state update` merely because an audit report
exists. A task is not complete until the verified change is committed and the next session can
recover the outcome from repository state.

## Fast Path For Editorial Tasks

Only `docs/editorial` tasks may use the fast path:

```text
intake
  -> classify
  -> minimal analysis
  -> implement
  -> verify
  -> commit
  -> durable state note
  -> close
```

Editorial fast path is allowed only for:

- spelling and grammar corrections
- dead-link fixes
- formatting cleanup
- wording clarification that does not change meaning
- comments or README adjustments with no behavior or governance effect

Editorial fast path is forbidden when the change:

- changes architecture meaning, API meaning, operational policy, or user expectations
- alters queue state, current-context state, acceptance criteria, or review rules
- mixes unrelated micro-tasks into one task or one commit
- discovers a hidden behavior change during execution

If an editorial task expands into meaning, contract, or governance change, stop treating it as fast
path and return to the canonical workflow immediately.

## TDD Requirements

TDD is mandatory for `feature dev`, `bug fix`, `refactor`, and `test/infrastructure` whenever the
task changes code or any behavior that can be verified.

Minimum cycle:

1. Write the failing test, fixture case, or reproducible check first.
2. Run it and confirm the failure is the expected one.
3. Implement the minimal change needed to pass.
4. Re-run the focused check until it passes.
5. Run the required regression or broader verification set before commit.

Additional rules:

- `bug fix` must reproduce the bug before the fix, using a test, fixture, schema case, or minimal
  reproduction command.
- `refactor` must prove no unintended behavior change using existing tests or added protection
  tests.
- `test/infrastructure` work that introduces new checker behavior must test the checker behavior
  first, not only the happy-path command.
- Architecture, governance, and documentation-only tasks are not forced into TDD, but they still
  require verification that matches their scope.

Writing implementation code before a failing test is a workflow violation unless the task is a true
editorial fast-path change.

## Commit Boundary Policy

GRCL Platform uses a strict task-bounded commit rule:

```text
one task -> one commit
```

Implications:

- verification and, when required, audit must pass before the task commit is created
- uncommitted long-lived task piles are not allowed
- if a task is too large for one coherent commit, split the task before implementation continues
- editorial fast-path tasks still commit independently
- audit rejection creates a new fix task with its own verification and its own commit

Commit messages should make the task type and intent clear enough for later audit, rollback, and
queue reconstruction.

## Durable State Update Rule

After the task commit, update the durable recovery surface that future sessions will read. The
minimum acceptable durable target depends on task type:

- architecture and governance tasks: update `docs/status/current-context.md`, the relevant index, or
  the governing design document entrypoint
- implementation tasks: update the task ledger, queue state, plan status, or current-context as
  appropriate
- editorial tasks: update the local note surface only when needed, but do not rely on chat history

At least one repository-resident recovery surface must reflect the result. A code or doc change
that leaves no durable recovery trace is incomplete.

## Relationship To Agentic Delivery

- Use this document first to decide task type and required gates.
- Use [Agentic Delivery Governance](agentic-delivery-governance.md) to execute implementation tasks
  with task briefs, implementation subagents, independent audit subagents, and ledgers.
- Use [Review And Verification Governance](review-and-verification-governance.md) to determine what
  evidence is sufficient for completion.

This means agentic delivery is a specialization of the broader task workflow, not a replacement for
task classification, TDD policy, commit boundaries, or durable recovery duties.
