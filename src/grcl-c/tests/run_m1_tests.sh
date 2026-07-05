#!/bin/sh

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
include_dir="$repo_root/src/grcl-c/include"
c_smoke_source="$repo_root/src/grcl-c/tests/compile_headers_smoke.c"
cpp_smoke_source="$repo_root/src/grcl-c/tests/compile_headers_smoke.cpp"
runtime_source="$repo_root/src/grcl-c/src/runtime.c"
null_backend_source="$repo_root/src/grcl-runtime-native/src/null_backend.c"
inprocess_backend_source="$repo_root/src/grcl-runtime-native/src/inprocess_backend.c"
runtime_lifecycle_source="$repo_root/src/grcl-c/tests/runtime_lifecycle_test.c"
backend_capability_source="$repo_root/src/grcl-c/tests/backend_capability_test.c"
diagnostics_negative_state_source="$repo_root/src/grcl-c/tests/diagnostics_negative_state_test.c"
object_ownership_source="$repo_root/src/grcl-c/tests/object_ownership_test.c"
pub_sub_source="$repo_root/src/grcl-c/tests/pub_sub_test.c"

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

output_root="$artifact_root/m1/grcl-c/tests"
mkdir -p "$output_root"

c_binary="$output_root/compile_headers_smoke_c11"
cpp_binary="$output_root/compile_headers_smoke_cpp17"
runtime_lifecycle_binary="$output_root/runtime_lifecycle_test"
backend_capability_binary="$output_root/backend_capability_test"
diagnostics_negative_state_binary="$output_root/diagnostics_negative_state_test"
object_ownership_binary="$output_root/object_ownership_test"
pub_sub_binary="$output_root/pub_sub_test"

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

run_step "c11 compile" \
  cc -std=c11 -I "$include_dir" "$c_smoke_source" -o "$c_binary"
run_step "c11 run" "$c_binary"

run_step "c++17 compile" \
  c++ -std=c++17 -I "$include_dir" "$cpp_smoke_source" -o "$cpp_binary"
run_step "c++17 run" "$cpp_binary"

run_step "runtime lifecycle compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" \
    "$runtime_lifecycle_source" -o "$runtime_lifecycle_binary"
run_step "runtime lifecycle run" "$runtime_lifecycle_binary"

run_step "backend capability compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" \
    "$backend_capability_source" -o "$backend_capability_binary"
run_step "backend capability run" "$backend_capability_binary"

run_step "diagnostics negative state compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" \
    "$diagnostics_negative_state_source" -o "$diagnostics_negative_state_binary"
run_step "diagnostics negative state run" "$diagnostics_negative_state_binary"

run_step "object ownership compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" \
    "$object_ownership_source" -o "$object_ownership_binary"
run_step "object ownership run" "$object_ownership_binary"

run_step "pub/sub compile" \
  cc -std=c11 -I "$include_dir" "$runtime_source" "$null_backend_source" \
    "$inprocess_backend_source" "$pub_sub_source" -o "$pub_sub_binary"
run_step "pub/sub run" "$pub_sub_binary"

printf 'PASS m1 test harness (%s)\n' "$output_root"
