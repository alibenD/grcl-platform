# ADR-0010 GRCL-C Capability ABI Representation

- Status: `accepted`
- Date: `2026-07-03`

## Decision

GRCL selects the recommended hybrid public C ABI strategy for runtime capability data in G2:

- fixed root structs for stable identity, protocol, profile/class, storage, lifecycle, result, and
  graph projection summary fields.
- descriptor structs, iterator-style APIs, or caller-provided output buffer APIs for variable
  capability families such as transports, QoS variants, encodings, diagnostics, and security
  subsets.
- no opaque serialized blob in G2 except as a future extension point for protocol, gateway,
  tooling, or management-plane work.

Capability, availability, and health remain separate concepts and must not be collapsed into a
single public ABI object.

## Context

The G1 runtime capability schema is nested and profile-dependent. A public C ABI cannot safely
export unbounded arrays, language-level containers, YAML/JSON parser objects, C++ types, ROS2
types, or schema-specific dynamic objects. It also cannot grow one large root struct for every
future capability family without creating ABI bloat and evolution risk.

The C ABI must be inspectable by direct C users, bindable by higher-level SDKs, and usable by
MCU/RTOS targets that require bounded storage and caller-controlled allocation.

## Consequences

- G2-B can continue header hardening using `struct_size`, `abi_version`, reserved enum values, and
  caller-visible buffer rules.
- G2-D must model capability query and negotiation through fixed roots plus descriptors,
  iterators, or caller-provided output buffers rather than a flattened capability mega-struct.
- Backend SPI capability hooks remain a G3 concern and are not defined by this ADR.
- Future management-plane or protocol work may introduce serialized capability payloads, but those
  payloads are extensions rather than the G2 public C ABI baseline.
- Conformance must keep testing that capability, availability, and health records remain separated.
