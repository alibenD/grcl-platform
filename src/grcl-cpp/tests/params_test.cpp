#include <cstddef>
#include <cstdio>
#include <cstring>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

#include <grcl/cpp/params.hpp>
#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace {

int failures = 0;

void report_failure(const char * label, const char * detail)
{
  std::fprintf(stderr, "%s: %s\n", label, detail);
  ++failures;
}

void expect_result(const char * label, grcl::Result actual, grcl::Result expected)
{
  if (actual != expected) {
    std::fprintf(
      stderr,
      "%s: expected %u, got %u\n",
      label,
      static_cast<unsigned>(expected),
      static_cast<unsigned>(actual));
    ++failures;
  }
}

void expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual != expected) {
    report_failure(label, "size mismatch");
  }
}

void expect_type(
  const char * label,
  ::grcl_param_type_t actual,
  ::grcl_param_type_t expected)
{
  if (actual != expected) {
    report_failure(label, "type mismatch");
  }
}

void expect_true(const char * label, bool condition)
{
  if (!condition) {
    report_failure(label, "expected true");
  }
}

void expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (std::memcmp(actual, expected, size) != 0) {
    report_failure(label, "bytes mismatch");
  }
}

bool contains_name(const char * names, size_t names_size, const char * expected_name)
{
  const size_t expected_size = std::strlen(expected_name);
  size_t start = 0u;

  while (start < names_size) {
    size_t end = start;
    while (end < names_size && names[end] != '\n') {
      ++end;
    }
    if ((end - start) == expected_size &&
      std::memcmp(names + start, expected_name, expected_size) == 0) {
      return true;
    }
    start = end + 1u;
  }

  return false;
}

void init_inprocess_runtime_options(::grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-params";
  options.profile_name = "native-inprocess";
}

::grcl_param_record_t make_param_record(
  const char * name,
  ::grcl_param_type_t type,
  const void * value,
  size_t value_size)
{
  ::grcl_param_record_t record{};
  record.struct_size = sizeof(record);
  record.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  record.name = name;
  record.type = type;
  record.value = value;
  record.value_size = value_size;
  return record;
}

int test_runtime_local_params_preserve_c_contract()
{
  ::grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  unsigned char enabled = 1u;
  char mode[] = "demo";
  grcl::Params params(&runtime);
  ::grcl_param_record_t enabled_record =
    make_param_record("feature.enabled", GRCL_PARAM_TYPE_BOOL, &enabled, sizeof(enabled));
  ::grcl_param_record_t mode_record =
    make_param_record("runtime.mode", GRCL_PARAM_TYPE_STRING, mode, sizeof(mode));
  ::grcl_param_record_t out_param{};
  unsigned char value_buffer[64u] = {0};
  char names_buffer[64u] = {0};
  char small_names_buffer[8u] = {0};
  size_t out_value_size = 0u;
  size_t out_names_size = 0u;
  size_t out_param_count = 0u;
  const unsigned char expected_enabled[] = {1u};
  const unsigned char expected_mode[] = {'d', 'e', 'm', 'o', '\0'};

  init_inprocess_runtime_options(runtime_create_options);
  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);

  expect_result("set bool before start", params.set(&enabled_record), GRCL_OK);
  enabled = 0u;

  expect_result("runtime start", runtime.start(), GRCL_OK);
  expect_result("set string while started", params.set(&mode_record), GRCL_OK);
  mode[0] = 'x';

  expect_result(
    "get bool",
    params.get(
      "feature.enabled",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_OK);
  expect_type("bool type", out_param.type, GRCL_PARAM_TYPE_BOOL);
  expect_true("bool value buffer forwarded", out_param.value == value_buffer);
  expect_size("bool value size", out_value_size, sizeof(expected_enabled));
  expect_bytes("bool copy-in bytes", value_buffer, expected_enabled, sizeof(expected_enabled));

  expect_result(
    "small string buffer capacity",
    params.get("runtime.mode", &out_param, value_buffer, 2u, &out_value_size),
    GRCL_ERROR_CAPACITY_EXCEEDED);
  expect_size("small string buffer required size", out_value_size, sizeof(expected_mode));

  std::memset(value_buffer, 0, sizeof(value_buffer));
  expect_result(
    "get string",
    params.get(
      "runtime.mode",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_OK);
  expect_type("string type", out_param.type, GRCL_PARAM_TYPE_STRING);
  expect_true("string value buffer forwarded", out_param.value == value_buffer);
  expect_size("string value size", out_value_size, sizeof(expected_mode));
  expect_bytes("string copy-in bytes", value_buffer, expected_mode, sizeof(expected_mode));

  expect_result(
    "small list buffer capacity",
    params.list(
      small_names_buffer,
      sizeof(small_names_buffer),
      &out_names_size,
      &out_param_count),
    GRCL_ERROR_CAPACITY_EXCEEDED);
  expect_size("small list param count", out_param_count, 2u);
  expect_true("small list needs more than buffer", out_names_size > sizeof(small_names_buffer));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result(
    "set bool after stop",
    params.set(&enabled_record),
    GRCL_OK);
  expect_result(
    "list after stop",
    params.list(names_buffer, sizeof(names_buffer), &out_names_size, &out_param_count),
    GRCL_OK);
  expect_size("list param count", out_param_count, 2u);
  expect_true(
    "list contains feature.enabled",
    contains_name(names_buffer, out_names_size, "feature.enabled"));
  expect_true(
    "list contains runtime.mode",
    contains_name(names_buffer, out_names_size, "runtime.mode"));
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  return failures;
}

int test_negative_results_forward_without_new_semantics()
{
  grcl::Runtime null_runtime;
  grcl::Params null_params(&null_runtime);
  const unsigned char enabled = 1u;
  ::grcl_param_record_t enabled_record =
    make_param_record("feature.enabled", GRCL_PARAM_TYPE_BOOL, &enabled, sizeof(enabled));
  ::grcl_param_record_t out_param{};
  unsigned char value_buffer[16u] = {0};
  char names_buffer[32u] = {0};
  size_t out_value_size = 0u;
  size_t out_names_size = 0u;
  size_t out_param_count = 0u;

  expect_result("null runtime create", grcl::Runtime::create(&null_runtime, nullptr), GRCL_OK);
  expect_result(
    "null backend set before start",
    null_params.set(&enabled_record),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result(
    "null backend get before start",
    null_params.get(
      "feature.enabled",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result(
    "null backend list before start",
    null_params.list(names_buffer, sizeof(names_buffer), &out_names_size, &out_param_count),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result("null runtime start", null_runtime.start(), GRCL_OK);
  expect_result(
    "null backend set while started",
    null_params.set(&enabled_record),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result("null runtime stop", null_runtime.stop(), GRCL_OK);
  expect_result(
    "null backend list after stop",
    null_params.list(names_buffer, sizeof(names_buffer), &out_names_size, &out_param_count),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result("null runtime destroy", null_runtime.destroy(), GRCL_OK);

  ::grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  grcl::Params params(&runtime);
  init_inprocess_runtime_options(runtime_create_options);
  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  expect_result("set null param", params.set(nullptr), GRCL_ERROR_INVALID_ARGUMENT);
  expect_result(
    "get empty name",
    params.get("", &out_param, value_buffer, sizeof(value_buffer), &out_value_size),
    GRCL_ERROR_INVALID_ARGUMENT);
  expect_result(
    "list null size pointer",
    params.list(names_buffer, sizeof(names_buffer), nullptr, &out_param_count),
    GRCL_ERROR_INVALID_ARGUMENT);
  expect_result(
    "get missing name",
    params.get(
      "missing.name",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_ERROR_NOT_FOUND);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  return failures;
}

}  // namespace

int main()
{
  failures = 0;
  test_runtime_local_params_preserve_c_contract();
  test_negative_results_forward_without_new_semantics();

  if (failures == 0) {
    std::puts("params_test: ok");
    return 0;
  }

  return 1;
}
