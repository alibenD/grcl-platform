# GRCL Schema Artifacts

This directory contains the first contract schema artifacts for GRCL Platform.
They are inputs to future conformance fixtures, C struct discussions, and wire
protocol design reviews. They do not implement runtime behavior.

Current schemas:

- `runtime-capability-record.schema.yaml` describes the stable capability record
  a runtime can declare before channels are opened.
- `mcu-profile.schema.yaml` describes profile descriptors for constrained,
  gateway, Linux, and simulator runtimes.
- `capability-negotiation-result.schema.yaml` describes visible accepted,
  degraded, and rejected negotiation outcomes.

Runtime capability, availability, and health remain separate concepts. This
task creates only the runtime capability record schema and the negotiation
result schema; it does not define runtime availability or health schemas.

