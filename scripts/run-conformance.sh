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

export GRCL_PLATFORM_ARTIFACT_ROOT="$artifact_root"

runner_output_root="$GRCL_PLATFORM_ARTIFACT_ROOT/g5/conformance"
mkdir -p "$runner_output_root"

stage_index=0

run_stage() {
  stage_index=$((stage_index + 1))
  stage_label=$1
  shift

  printf '==> [%s/6] %s\n' "$stage_index" "$stage_label"
  if "$@"; then
    printf 'PASS [%s/6] %s\n' "$stage_index" "$stage_label"
  else
    status=$?
    printf 'FAIL [%s/6] %s (exit %s)\n' "$stage_index" "$stage_label" "$status" >&2
    exit "$status"
  fi
}

cd "$repo_root"

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

summary_path="$runner_output_root/runner-summary.txt"
cat >"$summary_path" <<EOF
local conformance runner: ok
repository_root: $repo_root
artifact_root: $GRCL_PLATFORM_ARTIFACT_ROOT
stages:
- documentation checks
- runtime capability fixtures
- MCU profile fixtures
- C ABI/header checks
- M1 runnable harness
- SDK boundary drift checks
EOF

printf 'PASS conformance runner (%s)\n' "$summary_path"
