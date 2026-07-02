# ADR-0007 MCU Runtime Profile Contract

- Status: `draft`
- Date: `2026-07-02`

## Decision

MCU and RTOS targets implement declared profiles rather than full GRCL. Profile declarations define
allocation policy, graph depth, transport set, QoS subset, executor model, diagnostics, security,
entity limits, and payload limits.

## Rationale

Constrained runtimes cannot safely accept unbounded desktop runtime assumptions. Profile-based
contracts let peers discover constraints before channel establishment.

## Consequences

- Unsupported capabilities must fail fast.
- Full graph cache can be forbidden by profile.
- Static manifest and gateway-represented modes are first-class deployment options.

