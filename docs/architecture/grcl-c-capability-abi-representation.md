# GRCL-C Capability ABI Representation

## Purpose

This document records the public C ABI representation strategy for GRCL runtime capability data.
It resolves the G2 representation gate for runtime capability exchange without starting header
implementation, backend SPI implementation, SDK implementation, or management-plane work.

The selected strategy is the recommended hybrid:

- fixed root structs for stable, versioned, small-surface capability data.
- descriptor structs, iterator-style APIs, or caller-provided output buffer APIs for variable
  capability families.
- no opaque serialized blob in G2 except as a future extension point for protocol and
  management-plane work.

## Decision Boundary

Runtime capability data is not a single flat object in the platform architecture. The schema-level
model remains nested and profile-dependent, while the public C ABI must stay bounded, evolvable, and
usable by MCU/RTOS profiles and language bindings.

G2 therefore must not flatten every schema field into one giant public struct and must not expose
language containers, YAML/JSON parser objects, C++ types, ROS2 types, or unbounded dynamic arrays
through public headers.

## Fixed Root Struct Fields

Fixed root structs are reserved for stable identity and summary data that future callers can inspect
without negotiating a variable family first.

The G2 public root surface may include:

| Area | Root Data |
|---|---|
| identity | runtime identity, boot identity, domain, profile/class identifier, implementation identifier and version |
| protocol | GRCL protocol major/minor, capability schema version, ABI version |
| graph | supported graph projection mode summary and preferred/default projection mode |
| storage and limits | bounded storage flags, heap policy summary, max payload, max entity, endpoint, session, channel, and buffer summaries |
| lifecycle and result | lifecycle compatibility flags, negotiation status, scope, result category, diagnostic code identity |
| diagnostics and security summary | high-level support flags such as basic diagnostics or security family presence when the flag is only a summary |

All public structs crossing ABI boundaries must retain the existing `struct_size` and, where
version-bearing, `abi_version` compatibility rules.

## Variable Capability Families

Variable capability families must use descriptor structs, iterator-style APIs, or
caller-provided output buffer APIs. They must not be expanded into unbounded fields on the root
capability record.

This applies at least to:

| Family | Representation Direction |
|---|---|
| transports | descriptor entries or indexed query APIs for transport kind, MTU, fragmentation, retransmission, and flow-control details |
| QoS variants and subsets | descriptor entries or iterators for best-effort/reliable variants, history, deadline, liveliness, and durability subsets |
| encodings | descriptor entries or output-buffer queries for supported encodings, endian support, static/dynamic type support, and compression |
| diagnostics capabilities | descriptors for basic health, metrics, tracing, log sinks, and future diagnostic extensions |
| security capability subsets | descriptors for authentication, authorization, enclave, audit, signed-manifest, and future security families |

Descriptor structs must follow the same ABI compatibility style as other public structs: explicit
size/version fields where needed, reserved invalid enum values, and caller-visible count or
capacity behavior.

Iterator-style APIs must define stable iteration behavior, result handling, and caller ownership of
returned data. Caller-provided output buffer APIs must allow count discovery or capacity-checked
copying so constrained profiles can avoid hidden heap allocation.

## Record Separation

Capability, availability, and health remain separate concepts in G2:

- capability records describe what a runtime can support in principle.
- availability records describe current resource availability or pressure.
- health records describe heartbeat, lifecycle, fault, or degradation state.

G2 capability ABI work must not collapse capability, availability, and health into a single ABI
object. Capability negotiation may reference diagnostic codes and result status, but it must not
turn transient health or availability into static capability.

## Opaque Serialized Blob Deferral

Opaque serialized capability blobs are deferred from G2.

They remain a possible future extension point for wire protocol, management-plane snapshots,
gateway representation, or tooling exports, but they are not the active public C ABI baseline for
G2 because they would require serialization/versioning policy before the core ABI is ready and
would reduce direct inspectability for C and MCU/RTOS users.

G2 may reserve extension points for later serialized representations, but public G2 capability
queries must be expressible through fixed root structs plus descriptors, iterators, or
caller-provided output buffer APIs.

## Impact On G2 And Later Goals

G2-A resolves the GQ-030 representation gate and records the decision for later work.

G2-B may harden version, result, type, storage, allocator, and diagnostic headers using the ABI
rules above, but it must not implement capability query details beyond its assigned scope.

G2-D may add capability query and negotiation structs/functions consistent with this document, the
runtime capability schema, and the existing separation between capability, availability, and
health.

G3 must still define the backend SPI function-table shape and backend capability hooks. G2-A does
not define exact backend registration tables.

G10 may revisit opaque serialized blob support for management-plane snapshots or protocol-facing
representation, but that work must preserve the G2 public ABI baseline rather than replacing it
retroactively.

## Related Records

- [GRCL-C API Shape](grcl-c-api-shape.md)
- [Runtime Capability Schema](runtime-capability-schema.md)
- [Runtime Capability Graph](runtime-capability-graph.md)
- [Backend SPI Contract](backend-spi-contract.md)
- [ADR-0010 GRCL-C Capability ABI Representation](../adr/ADR-0010-grcl-c-capability-abi-representation.md)
