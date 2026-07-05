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

void expect_result(const char * label, grcl::Result actual, grcl::Result expected)
{
  if (actual != expected) {
    std::fprintf(
      stderr,
      "FAIL %s: expected %u got %u\n",
      label,
      static_cast<unsigned>(expected),
      static_cast<unsigned>(actual));
    ++failures;
  }
}

void expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual != expected) {
    std::fprintf(stderr, "FAIL %s: expected %zu got %zu\n", label, expected, actual);
    ++failures;
  }
}

void expect_type(
  const char * label,
  ::grcl_param_type_t actual,
  ::grcl_param_type_t expected)
{
  if (actual != expected) {
    std::fprintf(stderr, "FAIL %s: expected %u got %u\n", label, expected, actual);
    ++failures;
  }
}

void expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (std::memcmp(actual, expected, size) != 0) {
    std::fprintf(stderr, "FAIL %s: bytes mismatch\n", label);
    ++failures;
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

void init_runtime_options(::grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-params-example";
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

}  // namespace

int main()
{
  ::grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  unsigned char enabled = 1u;
  const char mode[] = "demo";
  ::grcl_param_record_t enabled_record =
    make_param_record("feature.enabled", GRCL_PARAM_TYPE_BOOL, &enabled, sizeof(enabled));
  ::grcl_param_record_t mode_record =
    make_param_record("runtime.mode", GRCL_PARAM_TYPE_STRING, mode, sizeof(mode));
  ::grcl_param_record_t out_param{};
  unsigned char value_buffer[64u] = {0};
  char names_buffer[64u] = {0};
  size_t out_value_size = 0u;
  size_t out_names_size = 0u;
  size_t out_param_count = 0u;
  grcl::Params params;

  init_runtime_options(runtime_create_options);
  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  params.reset(&runtime);

  expect_result("set enabled before start", params.set(&enabled_record), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);
  expect_result("set mode while started", params.set(&mode_record), GRCL_OK);

  expect_result(
    "get enabled",
    params.get(
      "feature.enabled",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_OK);
  expect_type("enabled type", out_param.type, GRCL_PARAM_TYPE_BOOL);
  expect_size("enabled size", out_value_size, sizeof(enabled));
  expect_bytes("enabled value", value_buffer, &enabled, sizeof(enabled));

  std::memset(value_buffer, 0, sizeof(value_buffer));
  expect_result(
    "get mode",
    params.get(
      "runtime.mode",
      &out_param,
      value_buffer,
      sizeof(value_buffer),
      &out_value_size),
    GRCL_OK);
  expect_type("mode type", out_param.type, GRCL_PARAM_TYPE_STRING);
  expect_size("mode size", out_value_size, sizeof(mode));
  expect_bytes("mode value", value_buffer, reinterpret_cast<const unsigned char *>(mode), sizeof(mode));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result(
    "list params",
    params.list(names_buffer, sizeof(names_buffer), &out_names_size, &out_param_count),
    GRCL_OK);
  expect_size("param count", out_param_count, 2u);
  if (!contains_name(names_buffer, out_names_size, "feature.enabled") ||
    !contains_name(names_buffer, out_names_size, "runtime.mode")) {
    std::fprintf(stderr, "FAIL list params: expected names missing\n");
    ++failures;
  }

  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  if (failures == 0) {
    return 0;
  }

  return 1;
}
