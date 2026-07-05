#!/bin/sh

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)

default_workspace_root=
if [ "$(basename -- "$(dirname -- "$repo_root")")" = "src" ]; then
  default_workspace_root=$(CDPATH= cd -- "$(dirname -- "$repo_root")/.." && pwd)
fi

artifact_root=${GRCL_PLATFORM_ARTIFACT_ROOT:-}
if [ -z "$artifact_root" ]; then
  if [ -n "$default_workspace_root" ]; then
    artifact_root="$default_workspace_root/artifacts"
  else
    printf '%s\n' \
      "FAIL artifact root resolution (set GRCL_PLATFORM_ARTIFACT_ROOT when the workspace layout is not <workspace>/src/grcl-platform)" >&2
    exit 1
  fi
fi

forbidden_artifact_root=
if [ -n "$default_workspace_root" ]; then
  forbidden_artifact_root="$default_workspace_root/src/artifacts"
fi

export GRCL_PLATFORM_ARTIFACT_ROOT="$artifact_root"

runner_output_root="$GRCL_PLATFORM_ARTIFACT_ROOT/g5/conformance"

stage_index=0
total_stages=10

run_stage() {
  stage_index=$((stage_index + 1))
  stage_label=$1
  shift

  printf '==> [%s/%s] %s\n' "$stage_index" "$total_stages" "$stage_label"
  if "$@"; then
    printf 'PASS [%s/%s] %s\n' "$stage_index" "$total_stages" "$stage_label"
  else
    status=$?
    printf 'FAIL [%s/%s] %s (exit %s)\n' "$stage_index" "$total_stages" "$stage_label" "$status" >&2
    exit "$status"
  fi
}

check_artifact_root_hygiene() {
  if [ -n "$forbidden_artifact_root" ] && [ -e "$forbidden_artifact_root" ]; then
    printf '%s\n' \
      "generated artifacts must use workspace-level artifacts/ or GRCL_PLATFORM_ARTIFACT_ROOT; forbidden path exists: $forbidden_artifact_root" >&2
    return 1
  fi
}

cd "$repo_root"

run_stage "artifact root hygiene" \
  check_artifact_root_hygiene

mkdir -p "$runner_output_root"

run_stage "documentation checks" \
  python3 scripts/check-docs.py

run_stage "runtime capability fixtures" \
  python3 scripts/check-schema-fixtures.py --runtime-capability

run_stage "MCU profile fixtures" \
  python3 scripts/check-schema-fixtures.py --mcu-profiles

run_stage "C ABI/header checks" \
  python3 scripts/check-c-abi.py

run_stage "M1 runnable harness" \
  src/grcl-c/tests/run_m1_tests.sh

run_stage "SDK boundary drift checks" \
  python3 scripts/check-sdk-boundaries.py

run_stage "M3 core middleware examples" \
  examples/c/run_m3_examples.sh

run_stage "M4 core contract tests" \
  src/grcl-c/tests/run_m4_contract_tests.sh

run_stage "M5 C++ local-core examples" \
  examples/cpp/run_m5_cpp_examples.sh

summary_path="$runner_output_root/runner-summary.txt"
cat >"$summary_path" <<EOF
local conformance runner: ok
repository_root: $repo_root
artifact_root: $GRCL_PLATFORM_ARTIFACT_ROOT
stages:
- artifact root hygiene
- documentation checks
- runtime capability fixtures
- MCU profile fixtures
- C ABI/header checks
- M1 runnable harness
- SDK boundary drift checks
- M3 core middleware examples
- M4 core contract tests
- M5 C++ local-core examples
EOF

printf 'PASS conformance runner (%s)\n' "$summary_path"
