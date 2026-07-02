# ADR-0006 Runtime Capability Schema

- Status: `draft`
- Date: `2026-07-02`

## Decision

Runtime compatibility is described through separate capability, availability, and health records.
Capability negotiation returns `accepted`, `degraded_accepted`, or `rejected_incompatible` with
machine-readable reasons.

## Rationale

Capability, availability, and health change at different rates and answer different questions.
Mixing them would make MCU profiles harder to reason about and would create unnecessary graph
churn.

## Consequences

- Wire protocol and management API must preserve this separation.
- Channel opening cannot silently downgrade requested behavior.
- Conformance tests must cover accepted, degraded, and rejected negotiation paths.

