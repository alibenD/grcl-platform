#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

static int expect_result(const char * label, grcl_result_t actual, grcl_result_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: result %d expected %d\n", label, actual, expected);
  return 1;
}

static int expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: size %zu expected %zu\n", label, actual, expected);
  return 1;
}

static int expect_type(
  const char * label,
  grcl_param_type_t actual,
  grcl_param_type_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: type %u expected %u\n", label, (unsigned)actual, (unsigned)expected);
  return 1;
}

static int expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (memcmp(actual, expected, size) == 0) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: bytes mismatch\n", label);
  return 1;
}

static int contains_name(const char * names, size_t names_size, const char * expected_name)
{
  size_t expected_size = strlen(expected_name);
  size_t start = 0u;

  while (start < names_size) {
    size_t end = start;
    while (end < names_size && names[end] != '\n') {
      ++end;
    }
    if ((end - start) == expected_size &&
      memcmp(names + start, expected_name, expected_size) == 0) {
      return 1;
    }
    start = end + 1u;
  }

  return 0;
}

static grcl_runtime_options_t runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "params-example";
  options.profile_name = "native-inprocess";
  return options;
}

static grcl_param_record_t param_record(
  const char * name,
  grcl_param_type_t type,
  const void * value,
  size_t value_size)
{
  grcl_param_record_t record = {0};
  record.struct_size = sizeof(record);
  record.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  record.name = name;
  record.type = type;
  record.value = value;
  record.value_size = value_size;
  return record;
}

int main(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_options_t runtime_create_options = runtime_options();
  const uint8_t enabled = 1u;
  static const char mode[] = "demo";
  grcl_param_record_t enabled_record = param_record(
    "feature.enabled",
    GRCL_PARAM_TYPE_BOOL,
    &enabled,
    sizeof(enabled));
  grcl_param_record_t mode_record = param_record(
    "runtime.mode",
    GRCL_PARAM_TYPE_STRING,
    mode,
    sizeof(mode));
  grcl_param_record_t out_param = {0};
  unsigned char value_buffer[64] = {0};
  char names_buffer[64] = {0};
  size_t value_size = 0u;
  size_t names_size = 0u;
  size_t param_count = 0u;
  int started = 0;
  int failures = 0;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(&runtime_create_options, &runtime),
      GRCL_OK) != 0 ||
    expect_result(
      "set enabled before start",
      grcl_runtime_param_set(runtime, &enabled_record),
      GRCL_OK) != 0 ||
    expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }
  started = 1;

  if (expect_result(
      "set mode while started",
      grcl_runtime_param_set(runtime, &mode_record),
      GRCL_OK) != 0 ||
    expect_result(
      "get enabled",
      grcl_runtime_param_get(
        runtime,
        "feature.enabled",
        &out_param,
        value_buffer,
        sizeof(value_buffer),
        &value_size),
      GRCL_OK) != 0 ||
    expect_type("enabled type", out_param.type, GRCL_PARAM_TYPE_BOOL) != 0 ||
    expect_size("enabled size", value_size, sizeof(enabled)) != 0 ||
    expect_bytes("enabled bytes", value_buffer, &enabled, sizeof(enabled)) != 0 ||
    expect_result(
      "get mode",
      grcl_runtime_param_get(
        runtime,
        "runtime.mode",
        &out_param,
        value_buffer,
        sizeof(value_buffer),
        &value_size),
      GRCL_OK) != 0 ||
    expect_type("mode type", out_param.type, GRCL_PARAM_TYPE_STRING) != 0 ||
    expect_size("mode size", value_size, sizeof(mode)) != 0 ||
    expect_bytes("mode bytes", value_buffer, (const unsigned char *)mode, sizeof(mode)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }
  started = 0;

  if (expect_result(
      "list params",
      grcl_runtime_param_list(
        runtime,
        names_buffer,
        sizeof(names_buffer),
        &names_size,
        &param_count),
      GRCL_OK) != 0 ||
    expect_size("param count", param_count, 2u) != 0) {
    failures = 1;
    goto cleanup;
  }

  if (!contains_name(names_buffer, names_size, "feature.enabled") ||
    !contains_name(names_buffer, names_size, "runtime.mode")) {
    (void)fprintf(stderr, "FAIL list params: expected names missing\n");
    failures = 1;
    goto cleanup;
  }

cleanup:
  if (runtime != NULL && started) {
    if (expect_result("stop runtime cleanup", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
      failures = 1;
    }
  }
  if (runtime != NULL) {
    if (expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK) != 0) {
      failures = 1;
    }
  }

  return failures;
}
