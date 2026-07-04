#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
workspace_root=$(CDPATH= cd -- "$repo_root/../.." && pwd)
artifact_root=${GRCL_PLATFORM_ARTIFACT_ROOT:-"$workspace_root/artifacts"}
output_root="$artifact_root/g6/grcl-cpp"

cc_bin=${CC:-cc}
cxx_bin=${CXX:-c++}

include_dir="$repo_root/src/grcl-c/include"
cpp_include_dir="$repo_root/src/grcl-cpp/include"
runtime_source="$repo_root/src/grcl-c/src/runtime.c"
null_backend_source="$repo_root/src/grcl-runtime-native/src/null_backend.c"
test_source="$repo_root/src/grcl-cpp/tests/runtime_wrapper_smoke.cpp"

runtime_object="$output_root/runtime.o"
backend_object="$output_root/null_backend.o"
test_object="$output_root/runtime_wrapper_smoke.o"
binary="$output_root/runtime_wrapper_smoke"

mkdir -p "$output_root"

run_step() {
  printf '%s\n' "$1"
  shift
  "$@"
}

run_step "compile grcl-c runtime" \
  "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
    -c "$runtime_source" -o "$runtime_object"

run_step "compile null backend" \
  "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
    -c "$null_backend_source" -o "$backend_object"

run_step "compile grcl-cpp smoke test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
    -c "$test_source" -o "$test_object"

run_step "link grcl-cpp smoke test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror \
    "$runtime_object" "$backend_object" "$test_object" -o "$binary"

run_step "run grcl-cpp smoke test" "$binary"
