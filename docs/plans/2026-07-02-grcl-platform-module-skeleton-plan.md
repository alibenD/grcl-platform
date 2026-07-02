# GRCL Platform Module Skeleton Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development
> (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** After user confirmation, create the first module skeletons, schema artifacts, and
verification scaffolding that turn the accepted architecture documents into an implementation-ready
platform repository.

**Architecture:** Keep `grcl-platform` as a modular monorepo. Add only skeletons, schemas, sample
test vectors, and verification scripts in the first implementation phase; do not implement runtime
behavior until these contracts are reviewed.

**Tech Stack:** Markdown, YAML, shell scripts, C header skeletons, future C/C++/Python/ROS2/RTOS
toolchains.

---

## Preconditions

- User has reviewed and accepted `docs/plans/2026-07-02-grcl-platform-architecture-plan.md`.
- User has accepted the modular monorepo decision in `docs/adr/ADR-0001-*`.
- User has accepted `grcl-c` as the core contract in `docs/adr/ADR-0002-*`.
- User has accepted runtime capability exchange in `docs/adr/ADR-0004-*`.

## Task 1: Create Module Skeleton Directories

**Files:**
- Create: `src/grcl-c/README.md`
- Create: `src/grcl-c/include/grcl/c/README.md`
- Create: `src/grcl-c/tests/README.md`
- Create: `src/grcl-cpp/README.md`
- Create: `src/grcl-py/README.md`
- Create: `src/grcl-idl/README.md`
- Create: `src/grcl-runtime-native/README.md`
- Create: `src/grcl-runtime-ros2/README.md`
- Create: `src/grcl-runtime-mcu/README.md`
- Create: `src/grcl-gateway/README.md`
- Create: `src/grcl-tools/README.md`

- [ ] **Step 1: Create `grcl-c` skeleton**

Create directories:

```bash
mkdir -p src/grcl-c/include/grcl/c src/grcl-c/tests
```

`src/grcl-c/README.md` must state:

```markdown
# grcl-c

`grcl-c` is the core C ABI and semantic contract for GRCL Platform.

Initial scope:

- runtime lifecycle
- node and endpoint ownership
- controlled storage
- result and error model
- capability query and negotiation
- backend SPI host boundary

No runtime behavior is implemented in this skeleton phase.
```

- [ ] **Step 2: Create language SDK skeleton READMEs**

Create `src/grcl-cpp/README.md`, `src/grcl-py/README.md`, and future SDK README entries that state
each SDK depends on `grcl-c` and must not define independent runtime semantics.

- [ ] **Step 3: Create runtime/backend skeleton READMEs**

Create native, ROS2, MCU, gateway, and tools README files with their accepted responsibilities from
`docs/architecture/backend-spi-contract.md`.

- [ ] **Step 4: Verify skeleton paths**

Run:

```bash
find src -maxdepth 3 -type f -name README.md -print | sort
```

Expected: all module README files listed in this task are present.

## Task 2: Add `grcl-c` Public Header Skeletons

**Files:**
- Create: `src/grcl-c/include/grcl/c/version.h`
- Create: `src/grcl-c/include/grcl/c/result.h`
- Create: `src/grcl-c/include/grcl/c/types.h`
- Create: `src/grcl-c/include/grcl/c/runtime.h`
- Create: `src/grcl-c/include/grcl/c/capability.h`
- Create: `src/grcl-c/include/grcl/c/storage.h`

- [ ] **Step 1: Add version constants**

`version.h` must define:

```c
#ifndef GRCL_C_VERSION_H_
#define GRCL_C_VERSION_H_

#define GRCL_C_ABI_VERSION_MAJOR 0u
#define GRCL_C_ABI_VERSION_MINOR 1u
#define GRCL_C_ABI_VERSION_PATCH 0u

#endif  /* GRCL_C_VERSION_H_ */
```

- [ ] **Step 2: Add result categories**

`result.h` must define the result enum categories from
`docs/architecture/grcl-c-api-shape.md`.

- [ ] **Step 3: Add opaque handles**

`types.h` must forward-declare the initial opaque handles:

```c
typedef struct grcl_runtime grcl_runtime_t;
typedef struct grcl_node grcl_node_t;
typedef struct grcl_endpoint grcl_endpoint_t;
typedef struct grcl_publisher grcl_publisher_t;
typedef struct grcl_subscription grcl_subscription_t;
typedef struct grcl_service grcl_service_t;
typedef struct grcl_client grcl_client_t;
typedef struct grcl_executor grcl_executor_t;
typedef struct grcl_type_support grcl_type_support_t;
typedef struct grcl_backend grcl_backend_t;
typedef struct grcl_transport grcl_transport_t;
typedef struct grcl_allocator grcl_allocator_t;
typedef struct grcl_storage grcl_storage_t;
```

- [ ] **Step 4: Add runtime API declarations**

`runtime.h` must declare create/start/stop/destroy and init-with-storage API shapes from
`docs/architecture/grcl-c-api-shape.md`.

- [ ] **Step 5: Add header hygiene check**

Run:

```bash
rg -n "rcl|rmw|rclcpp|std::|template|throw" src/grcl-c/include/grcl/c
```

Expected: no matches.

## Task 3: Add Schema Artifacts

**Files:**
- Create: `schemas/runtime-capability-record.schema.yaml`
- Create: `schemas/mcu-profile.schema.yaml`
- Create: `schemas/capability-negotiation-result.schema.yaml`
- Create: `schemas/README.md`

- [ ] **Step 1: Create schema directory**

Run:

```bash
mkdir -p schemas
```

- [ ] **Step 2: Encode runtime capability schema**

Create YAML schema sections matching
`docs/architecture/runtime-capability-schema.md`: identity, protocol, transport, QoS, encoding,
memory, executor, graph, diagnostics, and security.

- [ ] **Step 3: Encode MCU profile schema**

Create YAML schema fields matching `docs/architecture/mcu-profile-schema.md`.

- [ ] **Step 4: Encode negotiation result schema**

Create YAML schema fields for `accepted`, `degraded_accepted`, and `rejected_incompatible`.

## Task 4: Add Conformance Test Vector Skeletons

**Files:**
- Create: `tests/conformance/runtime-capability/accepted.yaml`
- Create: `tests/conformance/runtime-capability/degraded_accepted.yaml`
- Create: `tests/conformance/runtime-capability/rejected_incompatible.yaml`
- Create: `tests/conformance/mcu-profiles/baremetal-min.yaml`
- Create: `tests/conformance/mcu-profiles/rtos-basic.yaml`
- Create: `tests/conformance/mcu-profiles/rtos-posix-lite.yaml`

- [ ] **Step 1: Create test vector directories**

Run:

```bash
mkdir -p tests/conformance/runtime-capability tests/conformance/mcu-profiles
```

- [ ] **Step 2: Add negotiation examples**

Add one accepted, one degraded, and one rejected negotiation example consistent with
`docs/architecture/runtime-capability-schema.md`.

- [ ] **Step 3: Add MCU profile examples**

Add profile examples consistent with `docs/architecture/mcu-profile-schema.md`.

## Task 5: Add Development Environment Scaffolding

**Files:**
- Create: `scripts/env.sh`
- Create: `scripts/test-docs.sh`
- Create: `scripts/check-links.py`
- Create: `docker/ubuntu-ros-humble/README.md`
- Create: `docker/mcu-cross/README.md`

- [ ] **Step 1: Add shared environment helper**

`scripts/env.sh` must resolve the repository root and the artifact root without writing build
outputs into the source tree:

```bash
#!/usr/bin/env bash
set -euo pipefail

GRCL_PLATFORM_REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GRCL_PLATFORM_ARTIFACT_ROOT="${GRCL_PLATFORM_ARTIFACT_ROOT:-$(cd "$GRCL_PLATFORM_REPO_ROOT/.." && pwd)/.grcl-platform-artifacts}"

export GRCL_PLATFORM_REPO_ROOT
export GRCL_PLATFORM_ARTIFACT_ROOT
```

Expected default local artifact root when the repo is cloned at `/Users/aliben/Project/grcl-platform`:

```text
/Users/aliben/Project/.grcl-platform-artifacts
```

- [ ] **Step 2: Add docs test script**

`scripts/test-docs.sh` must run:

```bash
#!/usr/bin/env bash
set -euo pipefail
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/env.sh"
git diff --check
python3 scripts/check-links.py
rg -n "T[B]D|T[O]DO|fill[ ]in|implement[ ]later|place[ ]holder" README.md AGENTS.md docs docker scripts src tests manifests
```

The final `rg` command is expected to return no matches.

- [ ] **Step 3: Add internal link checker**

`scripts/check-links.py` must check relative Markdown links and exit non-zero on missing files.

- [ ] **Step 4: Add environment README files**

Document that Docker images are not implemented yet, and that macOS is host/orchestration while
Ubuntu containers are the ROS2 validation target.

- [ ] **Step 5: Document artifact root policy**

`docker/README.md` and `scripts/README.md` must state that build products go under
`GRCL_PLATFORM_ARTIFACT_ROOT`, not repository-root `build/`, `install/`, or `log/`.

- [ ] **Step 6: Add future build script requirements**

Future `scripts/test-native.sh` and `scripts/test-ros2.sh` must pass explicit colcon bases:

```bash
--build-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/build"
--install-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/install"
--log-base "$GRCL_PLATFORM_ARTIFACT_ROOT/colcon/native/log"
```

ROS2 variants must use the `colcon/ros2/` subtree. CMake variants must use `cmake/<module>/<mode>`
under the artifact root.

## Task 6: Verify And Commit

**Files:**
- Modify: `README.md`
- Modify: `docs/README.md`
- Modify: `docs/architecture/dependency-analysis.md`

- [ ] **Step 1: Update navigation**

Link schemas, module skeletons, and conformance vectors from top-level README or docs index.

- [ ] **Step 2: Run diff hygiene**

Run:

```bash
git diff --check
```

Expected: exit code 0 and no output.

- [ ] **Step 3: Run docs script**

Run:

```bash
bash scripts/test-docs.sh
```

Expected: exit code 0.

- [ ] **Step 4: Commit**

Run:

```bash
git add .
git commit -m "chore: add grcl platform module skeletons"
```

Expected: commit succeeds.

## Review Gate

This plan must not be executed until the user explicitly confirms the architecture baseline and the
module skeleton sequence.
