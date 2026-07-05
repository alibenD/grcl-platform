# Goal Execution Queue

## Active Goal

Drive GRCL Platform through dependency-ordered contract-artifact delivery with durable queue state,
task briefs, implementation subagents, and independent audit gates. G1 is closed. The authorized
continuous implementation window for G2-C through G2-E is also closed after audit acceptance. The
user-approved M1 Autonomous Goal Runner window for the first runnable `grcl-c` core with a
null/native-test backend is complete. G5 Conformance Harness v0.1 planning is complete, and the
user-approved G5 implementation window is complete after G5-G independent audit acceptance. G6
Language SDK Wrapper Skeletons is complete after G6-C/G6-D/G6-E independent audit acceptance and
G6-F closeout. M3 Core Middleware Contract And Native In-Process Backend is complete after M3-J
closeout audit acceptance. M4 GRCL-C Core Contract Stabilization And Conformance Expansion is
complete after M4-G closeout audit acceptance. The next active goal is M5 Local Core Product
Surface And GRCL-CPP Completion.

## Boundaries

- Workspace root: `/Users/aliben/Project/grcl-platform_ws`
- Repository root: `/Users/aliben/Project/grcl-platform_ws/src/grcl-platform`
- Code implementation: active only inside the approved M5 local-core product scope
- Completed work: M1 first runnable `grcl-c` core with null/native-test backend, using task briefs,
  implementation subagents, independent audit subagents, and TDD-style verification
- Allowed work now: M5 C++ conformance integration and closeout under GQ-092 is the next
  authorized batch; later M5 implementation batches remain dependency-ordered behind it
- Blocked outside M5 without a new user-approved goal window: ROS2, DDS, sockets, shared memory,
  multi-process transport, simulator backend, MCU runtime, gateway, management plane, auth, remote
  management, event streams, Docker, CI, external `grcl` migration, IDL/codegen, C++/Python
  example acceptance, and repo-wide build-system policy

## Carry-Forward Gate

- `GQ-010` from the previous iteration is superseded by `GQ-023`; the user authorized G1
  Autonomous Goal Runner execution, and G1 is now in closeout.
- Management-plane concept work is preserved as future input for G10, not as the current blocking
  path for core middleware progress.

## Queue

| ID | Task | Depends On | Status | Evidence |
|---|---|---|---|---|
| GQ-011 | Recover remaining architecture gates from durable docs | none | complete | `docs/status/current-context.md`, this queue, and management-related architecture docs read |
| GQ-012 | Select the next architecture-only closure scope | GQ-011 | complete | management plane identified from remaining gates in runtime layering, backend SPI, and SDK governance docs |
| GQ-013 | Write management-plane decision scope document | GQ-012 | complete | `docs/architecture/management-plane-decision-scope.md` |
| GQ-014 | Write dependency-ordered management-plane architecture plan | GQ-013 | complete | `docs/plans/2026-07-03-management-plane-architecture-plan.md` |
| GQ-015 | Update durable recovery state and navigation | GQ-013, GQ-014 | complete | `README.md`, `docs/README.md`, `docs/status/current-context.md`, this queue |
| GQ-016 | Reframe management-plane gate around concept-model review | GQ-015 | complete | user feedback incorporated; direct transport/auth/eventing/enforcement selection deferred |
| GQ-017 | Write management-plane concept and scope | GQ-016 | complete | `docs/architecture/management-plane-concept-and-scope.md` |
| GQ-018 | Update durable recovery state and decision scope for concept-first path | GQ-017 | complete | `docs/status/current-context.md`, `docs/architecture/management-plane-decision-scope.md`, README indexes |
| GQ-019 | Validate documentation hygiene for concept-model loop | GQ-018 | complete | `git diff --check`, document inventory, deferred-scope scan, and recovery-state scan passed |
| GQ-020 | Preserve management-plane concept as future G10 input | GQ-019 | complete | concept document remains available but no longer blocks G1 contract artifacts |
| GQ-021 | File middleware goal roadmap | GQ-020 | complete | `docs/status/middleware-goal-roadmap.md` |
| GQ-022 | Set G1 v0.1 Contract Artifacts as next active batch | GQ-021 | complete | this queue and `docs/status/current-context.md` |
| GQ-023 | Approve G1 implementation-stage task briefs and subagent execution | GQ-022 | complete | user authorized Autonomous Goal Runner mode; see `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-a-module-skeleton-directories/ledger.md` |
| GQ-024 | Execute G1-A module skeleton directories and README files | GQ-023 | complete | G1-A audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-a-module-skeleton-directories/audit-report.md` |
| GQ-025 | Execute G1-B `grcl-c` public header skeletons | GQ-024 | complete | G1-B audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-b-grcl-c-public-header-skeletons/audit-report.md` |
| GQ-026 | Execute G1-C schema artifacts | GQ-025 | complete | G1-C audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-c-schema-artifacts/audit-report.md` |
| GQ-027 | Execute G1-D conformance fixtures | GQ-026 | complete | G1-D audit status `accepted` in `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-d-conformance-fixtures/audit-report.md` |
| GQ-028 | Execute G1-E documentation and validation closeout | GQ-027 | complete | G1-E audit status `accepted` in `.local/agentic-runs/2026-07-02-grcl-platform-module-skeleton-plan/g1-e-documentation-validation-closeout/audit-report.md` |
| GQ-029 | Create and review a G2-specific plan before any G2 execution | GQ-028 | complete | `docs/plans/2026-07-03-grcl-c-abi-contract-plan.md` |
| GQ-030 | Resolve G2 runtime capability C ABI representation gate | GQ-029 | complete | user confirmed the recommended hybrid; recorded in `docs/architecture/grcl-c-capability-abi-representation.md` and `docs/adr/ADR-0010-grcl-c-capability-abi-representation.md` |
| GQ-031 | Create G2-A task brief after representation decision | GQ-030 | complete | `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-a-capability-abi-representation/task-brief.md` |
| GQ-032 | Complete G2-A implementation closeout and independent audit | GQ-031 | complete | G2-A audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-a-capability-abi-representation/audit-report.md` |
| GQ-033 | Prepare G2-B header-hardening task brief | GQ-032 | complete | `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-b-header-hardening/task-brief.md` |
| GQ-034 | Authorize G2-B header-hardening execution from the prepared brief | GQ-033 | complete | user approved `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-b-header-hardening/task-brief.md` |
| GQ-035 | Execute G2-B header hardening and independent audit | GQ-034 | complete | G2-B audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-b-header-hardening/audit-report.md` |
| GQ-036 | Prepare G2-C runtime lifecycle and endpoint declaration task brief | GQ-035 | complete | `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-c-runtime-lifecycle-and-endpoint-declarations/task-brief.md` |
| GQ-037 | Authorize continuous goal-driven execution for G2-C through G2-E | GQ-036 | complete | user approved goal-driven continuous execution instead of per-batch confirmation |
| GQ-038 | Execute G2-C runtime lifecycle and endpoint declaration hardening | GQ-037 | complete | G2-C audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-c-runtime-lifecycle-and-endpoint-declarations/audit-report.md` |
| GQ-039 | Execute G2-D capability query and negotiation ABI shaping | GQ-038 | complete | G2-D audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-d-capability-query-and-negotiation-abi/audit-report.md` |
| GQ-040 | Execute G2-E compile-only smoke and conformance closeout | GQ-039 | complete | G2-E audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-03-grcl-c-abi-contract-plan/g2-e-compile-only-smoke-and-conformance-closeout/audit-report.md` |
| GQ-041 | Complete middleware documentation IA cleanup before post-G2-E execution | GQ-040 | complete | `docs/README.md`, `docs/architecture/system-overview.md`, and documentation governance rules updated; no auto-advance into `G2-F` or `G3` |
| GQ-042 | Authorize G2-F closeout and G3 Backend SPI API Shape v0.1 design baseline | GQ-041 | complete | user approved design-only goal; no runtime/backend implementation authorized |
| GQ-043 | Close G2-F `grcl-c` ABI v0.1 baseline | GQ-042 | complete | `docs/architecture/grcl-c-api-shape.md` records G2-F closeout baseline and G4 minimum handoff |
| GQ-044 | Produce G3 Backend SPI API Shape v0.1 design baseline | GQ-043 | complete | `docs/architecture/backend-spi-contract.md` defines SPI ownership, operation-table shape, lifecycle/capability hooks, and G4 minimum target |
| GQ-045 | Authorize M1 First Runnable GRCL-C Core With Null/Native Backend | GQ-044 | complete | user approved M1 Autonomous Goal Runner with implementation subagents, independent audit, and auto-advance inside M1 |
| GQ-046 | File M1 implementation plan and durable queue | GQ-045 | complete | `docs/plans/2026-07-04-m1-first-runnable-core-plan.md`; queue updated for M1-A through M1-G |
| GQ-047 | M1-B add backend SPI header and compile-only coverage | GQ-046 | complete | M1-B audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-b-backend-spi-header/audit-report.md` |
| GQ-048 | M1-C add minimal local M1 test harness | GQ-047 | complete | M1-C audit status `accepted` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-c-local-test-harness/audit-report.md` |
| GQ-049 | M1-D implement core runtime lifecycle skeleton | GQ-048 | complete | M1-D audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-d-core-runtime-lifecycle/audit-report.md` |
| GQ-050 | M1-E implement null/native-test backend capability behavior | GQ-049 | complete | M1-E audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-e-null-native-backend-capability/audit-report.md` |
| GQ-051 | M1-F add diagnostics and negative state tests | GQ-050 | complete | M1-F audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-f-diagnostics-negative-states/audit-report.md` |
| GQ-052 | M1-G run full M1 verification and close runnable-core milestone | GQ-051 | complete | M1-G audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m1-first-runnable-core/m1-g-closeout/audit-report.md`; full local M1 harness passed from workspace root, with `GRCL_PLATFORM_ARTIFACT_ROOT` override, and from repository root; forbidden-scope and repo-root artifact scans passed |
| GQ-053 | Authorize G5 Conformance Harness v0.1 planning | GQ-052 | complete | user approved G5 planning for local contract-drift checks covering docs, schemas, C ABI, capability fixtures, MCU profile fixtures, and M1 harness integration |
| GQ-054 | File G5 goal-specific plan | GQ-053 | complete | `docs/plans/2026-07-04-g5-conformance-harness-plan.md` |
| GQ-055 | Update conformance design home and durable status for G5 planning | GQ-054 | complete | `docs/architecture/conformance-and-ci-strategy.md`, `docs/status/current-context.md`, `docs/status/middleware-goal-roadmap.md`, this queue, README navigation |
| GQ-056 | Validate and commit G5 planning closeout | GQ-055 | complete | `git diff --check`, unfinished-marker scan, stale-status scan, and changed-file scope scan passed for the G5 planning files |
| GQ-057 | Authorize G5 implementation window | GQ-056 | complete | user selected option 2 and approved goal-driven G5 implementation under the local contract-drift scope |
| GQ-058 | Execute G5-B documentation checks and G5-C runtime capability fixture checks | GQ-057 | complete | G5-B and G5-C audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-04-g5-conformance-harness/`; docs and runtime capability fixture checks pass |
| GQ-059 | Execute G5-D MCU profile fixture checks and G5-E C ABI/header checks | GQ-058 | complete | G5-D audit status `accepted_with_notes`; G5-E audit status `accepted` after narrow raw `ROS2_*` / `ROS_*` identifier fix |
| GQ-060 | Execute G5-F top-level local conformance runner | GQ-059 | complete | G5-F audit status `accepted_with_notes`; local runner composes docs, runtime capability, MCU profile, C ABI, and M1 harness stages |
| GQ-061 | Execute G5-G closeout verification and independent audit | GQ-060 | complete | G5-G audit status `accepted`; full local conformance runner passed from workspace root, with `/tmp` artifact override, and from repository root |
| GQ-062 | Plan G6 Language SDK Wrapper Skeletons / M2 boundary baseline | GQ-061 | complete | `docs/plans/2026-07-04-g6-language-sdk-wrapper-skeletons-plan.md`, SDK design home, roadmap, status, and navigation updated; no SDK code implemented |
| GQ-063 | Choose G6 Python binding substrate before implementation | GQ-062 | complete | user selected Option A: `ctypes`/private dynamic-library shim boundary with package distribution deferred |
| GQ-064 | Execute G6-C `grcl-cpp` minimal RAII boundary skeleton | GQ-063 | complete | G6-C audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/g6-c-grcl-cpp-raii-boundary/audit-report.md` |
| GQ-065 | Execute G6-D `grcl-py` minimal ownership boundary skeleton | GQ-063 | complete | G6-D audit status `accepted` in `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/g6-d-grcl-py-ownership-boundary/audit-report.md` |
| GQ-066 | Execute G6-E SDK boundary drift checks and conformance integration | GQ-064, GQ-065 | complete | G6-E audit status `accepted` in `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/g6-e-sdk-boundary-drift-checks/audit-report.md`; local runner now includes SDK boundary drift checks |
| GQ-067 | Execute G6-F closeout verification and durable status update | GQ-066 | complete | G6-F audit status `accepted` in `.local/agentic-runs/2026-07-04-g6-language-sdk-wrapper-skeletons/g6-f-closeout/audit-report.md`; full closeout verification passed |
| GQ-068 | File M3 core middleware native backend plan and design baseline | GQ-067 | complete | `docs/plans/2026-07-04-m3-core-middleware-native-backend-plan.md`; canonical design docs updated; `git diff --check` and `python3 scripts/check-docs.py` passed |
| GQ-069 | Run M3 architecture, ABI, and test-plan design reviews | GQ-068 | complete | architecture review `accepted_with_notes`; ABI/SPI re-review `accepted_with_notes`; test-plan re-review `accepted` under `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/` |
| GQ-070 | Execute M3-C API and SPI header contract batch | GQ-069 | complete | M3-C audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-c-api-spi-header-contract/audit-report.md`; header smoke and M1 harness passed |
| GQ-071 | Execute M3-D core object ownership batch | GQ-070 | complete | M3-D audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-d-core-object-ownership/audit-report.md`; object ownership tests and M1 harness passed; non-blocking runtime create-failure cleanup hardening noted |
| GQ-072 | Execute M3-E pub/sub routing batch | GQ-071 | complete | M3-E re-audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-e-pub-sub-routing/reaudit-report.md`; pub/sub tests and M1 harness passed |
| GQ-073 | Execute M3-F service/client routing batch | GQ-072 | complete | M3-F audit status `accepted_with_notes` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-f-service-client-routing/audit-report.md`; service/client tests, M1 harness, and ASan smoke passed |
| GQ-074 | Execute M3-G local runtime params batch | GQ-073 | complete | M3-G audit status `accepted` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-g-local-runtime-params/audit-report.md`; params tests and M1 harness passed |
| GQ-075 | Execute M3-H native backend capability update batch | GQ-074 | complete | M3-H audit status `accepted` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-h-native-backend-capability-update/audit-report.md`; capability tests and M1 harness passed |
| GQ-076 | Execute M3-I C examples batch | GQ-075 | complete | M3-I re-audit status `accepted` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-i-c-examples/reaudit-report.md`; all C examples passed from repo root, workspace root, and artifact-root override |
| GQ-077 | Execute M3-J conformance integration and closeout | GQ-076 | complete | M3-J closeout audit status `accepted` in `.local/agentic-runs/2026-07-04-m3-core-middleware-native-backend/m3-j-conformance-closeout/audit-report.md`; top-level local conformance runner now includes the M3 core middleware examples stage |
| GQ-078 | File M4 GRCL-C core contract stabilization plan and durable queue | GQ-077 | complete | `docs/plans/2026-07-05-m4-grcl-c-core-contract-stabilization-plan.md`; recovery docs and navigation updated; `git diff --check` and `python3 scripts/check-docs.py` passed |
| GQ-079 | Run M4 API, backend containment, and test-plan design reviews | GQ-078 | complete | API re-review `accepted`; backend containment and test-plan reviews `accepted_with_notes`; destroyed runtime UAF boundary fixed in M4 plan and `grcl-c` API design home |
| GQ-080 | Execute M4-C core lifecycle and ownership contract tests | GQ-079 | complete | audit status `accepted`; `runtime.c` undersized options drift fixed; M4 runner added; M1 harness and M3 examples remained green |
| GQ-081 | Execute M4-D messaging and executor contract tests | GQ-080 | complete | audit status `accepted_with_notes`; bounded `spin_once` contract implemented; non-blocking note to consider a response-type-only mismatch assertion later |
| GQ-082 | Execute M4-E runtime params and capability contract tests | GQ-081 | complete | audit status `accepted`; params/capability contract tests added to M4 runner; no production source changes required |
| GQ-083 | Execute M4-F local conformance integration | GQ-082 | complete | audit status `accepted`; top-level local conformance runner now includes 9 stages with M4 contract tests after M3 examples |
| GQ-084 | Execute M4-G closeout verification and durable status update | GQ-083 | complete | closeout audit status `accepted_with_notes`; full repo/workspace/override conformance, M4 runner, M1 harness, M3 examples, SDK boundary, docs, diff hygiene, and artifact scan passed |
| GQ-085 | Authorize M5 Local Core Product Surface And GRCL-CPP Completion | GQ-084 | complete | user set the current goal to fully complete `grcl-c` local core plus full `grcl-cpp` wrapping, passing core tests, and runnable examples |
| GQ-086 | File M5 goal-specific plan and durable queue state | GQ-085 | complete | `docs/plans/2026-07-05-m5-local-core-product-and-grcl-cpp-completion-plan.md`, current context, this queue, roadmap, and navigation updated |
| GQ-087 | Run M5 design closure and review baseline | GQ-086 | complete | canonical design homes updated; API review `accepted_with_notes`; C++ wrapper review `accepted_with_notes`; test-plan review `rejected` then design fix plus re-review `accepted` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/` |
| GQ-088 | Execute M5 `grcl-c` local-core surface closeout | GQ-087 | complete | implementation report concluded no C-side source change required; full local verification ladder passed; independent audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-c-grcl-c-local-core-closeout/` |
| GQ-089 | Execute M5 `grcl-cpp` runtime, node, and executor wrappers | GQ-088 | complete | implementation report and independent audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-d-grcl-cpp-runtime-node-executor/`; G6 smoke, new M5 C++ runner, M1 harness, M4 contract runner, docs check, diff hygiene, and top-level conformance passed |
| GQ-090 | Execute M5 `grcl-cpp` pub/sub wrappers and example | GQ-089 | complete | implementation report plus independent audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-e-grcl-cpp-pub-sub/`; G6 smoke, M5 C++ runner, M1 harness, M4 contract runner, C++ example runner, docs check, diff hygiene, and top-level conformance passed |
| GQ-091 | Execute M5 `grcl-cpp` service/client and params wrappers | GQ-090 | complete | M5-F1 service/client wrappers completed with implementation report plus independent audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-f-service-client/`; M5-F2 params wrappers completed with implementation report plus independent audit status `accepted_with_notes` under `.local/agentic-runs/2026-07-05-m5-local-core-product-and-grcl-cpp-completion/m5-f-params/` |
| GQ-092 | Execute M5 C++ conformance integration and closeout | GQ-091 | pending | top-level conformance integration, closeout verification, independent audit, and final M5 durable status closeout |

## Execution Rules

- Do not proceed from architecture/planning to implementation without explicit user approval.
- G1 implementation may create module skeletons, header skeletons, schemas, and conformance
  fixtures only after task briefs are created and execution is approved.
- Do not implement runtime behavior, backend behavior, SDK behavior, management plane, auth,
  remote management, event streams, or repo migration during G1.
- Do not treat G1 authorization or G2-A completion as permission to start G2-B or G2-D header
  implementation; G2-A must close through independent audit first.
- The previous continuous-execution window covered G2-C through G2-E only and is now complete.
- G2-F and G3 design baseline work was separately authorized as documentation-only work.
- M1 Autonomous Goal Runner is complete. It may not auto-advance into any post-M1 implementation
  milestone without a new user-approved goal window.
- G5 Conformance Harness v0.1 implementation is complete and no longer the active implementation
  window.
- G6 Language SDK Wrapper Skeletons implementation is complete. It does not authorize CI, Docker,
  repo-wide build-system changes, package distribution, generated shared libraries, native
  extension framework selection, pub/sub, graph, transport, ROS2, MCU runtime, simulator,
  management-plane implementation, auth, remote management, event streams, or external `grcl`
  migration.
- M3 Core Middleware Contract And Native In-Process Backend is complete. It delivered the approved
  M3 scope: in-process native backend, C API/SPI contract, executor pull, pub/sub bytes,
  service/client bytes, local runtime params, C tests, C examples, and local conformance
  integration.
- M4 GRCL-C Core Contract Stabilization And Conformance Expansion is complete. It delivered
  contract-hardening tests for the completed M3 `grcl-c` surface, narrow fixes driven by failing
  contract tests, local conformance expansion, and closeout. M4 did not authorize or introduce new
  public feature areas, ROS2, networking, simulator, MCU runtime, management plane, auth, remote
  management, event streams, Docker, CI, package/build-system rollout, external `grcl` migration,
  IDL/codegen, or C++/Python example acceptance.
- M5 Local Core Product Surface And GRCL-CPP Completion is now the active milestone. It is limited
  to completing the native-backend local-core product surface in `grcl-c`, fully wrapping that
  surface in `grcl-cpp`, and verifying it with tests plus runnable C/C++ examples.
- If a future task becomes implementation work, use file-based task briefs and independent audit
  subagents according to `docs/architecture/agentic-delivery-governance.md`.
- If validation fails, update this queue with the failure and fix task before claiming completion.
- If a decision cannot be made from repository documents, stop and ask the user rather than
  inventing policy.

## Current Decision State

G1 v0.1 Contract Artifacts is complete: G1-A through G1-E passed independent audit. Management-plane
work is deferred to G10 unless the user explicitly reprioritizes it. The G2-specific plan is filed,
GQ-030 is resolved by the recommended hybrid capability ABI decision, and G2-A through G2-F is
closed. G3 Backend SPI API Shape v0.1 design baseline is recorded in the canonical backend SPI
document. M1 first runnable `grcl-c` core with null/native-test backend is complete: M1-B through
M1-F passed independent audit, and M1-G passed independent audit with notes after verifying the full
local harness, artifact-root behavior, report/audit inventory, and forbidden-scope scans. G5
Conformance Harness v0.1 is complete after G5-G independent audit acceptance. G6 Language SDK
Wrapper Skeletons is complete after G6-C/G6-D/G6-E independent audit acceptance and G6-F closeout
verification. M3 Core Middleware Contract And Native In-Process Backend is complete under GQ-068
through GQ-077. M4 GRCL-C Core Contract Stabilization And Conformance Expansion is complete under
GQ-078 through GQ-084. M5 Local Core Product Surface And GRCL-CPP Completion is authorized under
GQ-085. GQ-086 through GQ-091 are complete. GQ-092 is now the next dependency-ordered batch.
