#!/bin/sh

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
include_dir="$repo_root/src/grcl-c/include"
runtime_source="$repo_root/src/grcl-c/src/runtime.c"
null_backend_source="$repo_root/src/grcl-runtime-native/src/null_backend.c"
inprocess_backend_source="$repo_root/src/grcl-runtime-native/src/inprocess_backend.c"
contract_test_source="$repo_root/src/grcl-c/tests/core_lifecycle_contract_test.c"

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

output_root="$artifact_root/m4/grcl-c/tests"
mkdir -p "$output_root"

contract_binary="$output_root/core_lifecycle_contract_test"

run_step() {
  step_name=$1
  shift
  if "$@"; then
    printf 'PASS %s\n' "$step_name"
  else
    status=$?
    printf 'FAIL %s (exit %s)\n' "$step_name" "$status" >&2
    exit "$status"
  fi
}

run_step "m4 lifecycle contract compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" "$contract_test_source" -o "$contract_binary"

run_step "m4 lifecycle contract run" "$contract_binary"

printf 'PASS m4 contract harness (%s)\n' "$output_root"
