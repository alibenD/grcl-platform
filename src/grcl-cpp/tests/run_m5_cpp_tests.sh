#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
workspace_root=$(CDPATH= cd -- "$repo_root/../.." && pwd)
artifact_root=${GRCL_PLATFORM_ARTIFACT_ROOT:-"$workspace_root/artifacts"}
output_root="$artifact_root/m5/grcl-cpp"

cc_bin=${CC:-cc}
cxx_bin=${CXX:-c++}

include_dir="$repo_root/src/grcl-c/include"
cpp_include_dir="$repo_root/src/grcl-cpp/include"
runtime_source="$repo_root/src/grcl-c/src/runtime.c"
null_backend_source="$repo_root/src/grcl-runtime-native/src/null_backend.c"
inprocess_backend_source="$repo_root/src/grcl-runtime-native/src/inprocess_backend.c"
runtime_node_executor_source="$repo_root/src/grcl-cpp/tests/runtime_node_executor_test.cpp"
pub_sub_test_source="$repo_root/src/grcl-cpp/tests/pub_sub_test.cpp"
service_client_test_source="$repo_root/src/grcl-cpp/tests/service_client_test.cpp"
params_test_source="$repo_root/src/grcl-cpp/tests/params_test.cpp"

runtime_object="$output_root/runtime.o"
backend_object="$output_root/null_backend.o"
inprocess_backend_object="$output_root/inprocess_backend.o"
runtime_test_object="$output_root/runtime_node_executor_test.o"
runtime_binary="$output_root/runtime_node_executor_test"
pub_sub_test_object="$output_root/pub_sub_test.o"
pub_sub_binary="$output_root/pub_sub_test"
service_client_test_object="$output_root/service_client_test.o"
service_client_binary="$output_root/service_client_test"
params_test_object="$output_root/params_test.o"
params_test_binary="$output_root/params_test"

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

run_step "compile inprocess backend" \
  "$cc_bin" -std=c11 -Wall -Wextra -Werror -I "$include_dir" \
    -c "$inprocess_backend_source" -o "$inprocess_backend_object"

run_step "compile grcl-cpp runtime/node/executor test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
    -c "$runtime_node_executor_source" -o "$runtime_test_object"

run_step "link grcl-cpp runtime/node/executor test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror \
    "$runtime_object" "$backend_object" "$inprocess_backend_object" "$runtime_test_object" -o "$runtime_binary"

run_step "run grcl-cpp runtime/node/executor test" "$runtime_binary"

run_step "compile grcl-cpp pub/sub test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
    -c "$pub_sub_test_source" -o "$pub_sub_test_object"

run_step "link grcl-cpp pub/sub test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror \
    "$runtime_object" "$backend_object" "$inprocess_backend_object" "$pub_sub_test_object" -o "$pub_sub_binary"

run_step "run grcl-cpp pub/sub test" "$pub_sub_binary"

run_step "compile grcl-cpp service/client test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
    -c "$service_client_test_source" -o "$service_client_test_object"

run_step "link grcl-cpp service/client test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror \
    "$runtime_object" "$backend_object" "$inprocess_backend_object" "$service_client_test_object" -o "$service_client_binary"

run_step "run grcl-cpp service/client test" "$service_client_binary"

run_step "compile grcl-cpp params test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror -I "$include_dir" -I "$cpp_include_dir" \
    -c "$params_test_source" -o "$params_test_object"

run_step "link grcl-cpp params test" \
  "$cxx_bin" -std=c++17 -Wall -Wextra -Werror \
    "$runtime_object" "$backend_object" "$inprocess_backend_object" "$params_test_object" -o "$params_test_binary"

run_step "run grcl-cpp params test" "$params_test_binary"
