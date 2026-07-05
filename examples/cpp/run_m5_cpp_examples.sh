#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd)
workspace_root=$(CDPATH= cd -- "$repo_root/../.." && pwd)
artifact_root=${GRCL_PLATFORM_ARTIFACT_ROOT:-"$workspace_root/artifacts"}
output_root="$artifact_root/m5/examples/cpp"

cc_bin=${CC:-cc}
cxx_bin=${CXX:-c++}

include_dir="$repo_root/src/grcl-c/include"
cpp_include_dir="$repo_root/src/grcl-cpp/include"
runtime_source="$repo_root/src/grcl-c/src/runtime.c"
null_backend_source="$repo_root/src/grcl-runtime-native/src/null_backend.c"
inprocess_backend_source="$repo_root/src/grcl-runtime-native/src/inprocess_backend.c"

mkdir -p "$output_root"

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

compile_and_run_example() {
  example_name=$1
  source_file="$repo_root/examples/cpp/${example_name}.cpp"
  binary_file="$output_root/$example_name"

  run_step "${example_name} compile" \
    "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
      -c "$runtime_source" -o "$output_root/runtime.o"

  run_step "${example_name} null backend compile" \
    "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
      -c "$null_backend_source" -o "$output_root/null_backend.o"

  run_step "${example_name} inprocess backend compile" \
    "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
      -c "$inprocess_backend_source" -o "$output_root/inprocess_backend.o"

  run_step "${example_name} cpp compile" \
    "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
      "$source_file" \
      "$output_root/runtime.o" \
      "$output_root/null_backend.o" \
      "$output_root/inprocess_backend.o" \
      -o "$binary_file"

  run_step "${example_name} run" "$binary_file"
}

compile_and_run_example "pub_sub_example"
compile_and_run_example "service_client_example"

printf 'PASS m5 cpp example harness (%s)\n' "$output_root"
