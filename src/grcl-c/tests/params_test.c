#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

static int expect_result(
  const char * label,
  grcl_result_t actual,
  grcl_result_t expected)
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

static int expect_u32(const char * label, uint32_t actual, uint32_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: value %u expected %u\n", label, actual, expected);
  return 1;
}

static int expect_ptr(const char * label, const void * actual, const void * expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: pointer mismatch\n", label);
  return 1;
}

static int expect_string(const char * label, const char * actual, const char * expected)
{
  if (actual != NULL && expected != NULL && strcmp(actual, expected) == 0) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: string mismatch\n", label);
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

  (void)fprintf(stderr, "FAIL %s: bytes differ\n", label);
  return 1;
}

static grcl_runtime_options_t inprocess_runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "params-test";
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

static int create_inprocess_runtime(grcl_runtime_t ** runtime)
{
  grcl_runtime_options_t options = inprocess_runtime_options();

  *runtime = NULL;
  if (grcl_runtime_create(&options, runtime) != GRCL_OK) {
    (void)fprintf(stderr, "FAIL create native in-process runtime\n");
    return 1;
  }

  return 0;
}

static int expect_param_get_success(
  grcl_runtime_t * runtime,
  const char * name,
  grcl_param_type_t expected_type,
  const unsigned char * expected_value,
  size_t expected_value_size)
{
  grcl_param_record_t out_param = {0};
  unsigned char value_buffer[256] = {0};
  size_t out_value_size = 0u;

  if (expected_value_size > sizeof(value_buffer)) {
    (void)fprintf(stderr, "FAIL helper value buffer too small\n");
    return 1;
  }

  if (expect_result(
      "get param",
      grcl_runtime_param_get(
        runtime,
        name,
        &out_param,
        value_buffer,
        sizeof(value_buffer),
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_size("out param struct size", out_param.struct_size, sizeof(out_param)) != 0 ||
    expect_u32("out param abi version", out_param.abi_version, GRCL_C_ABI_VERSION_CURRENT) != 0 ||
    expect_string("out param name", out_param.name, name) != 0 ||
    expect_u32("out param type", (uint32_t)out_param.type, (uint32_t)expected_type) != 0 ||
    expect_ptr("out param value pointer", out_param.value, value_buffer) != 0 ||
    expect_size("out param value size", out_param.value_size, expected_value_size) != 0 ||
    expect_size("reported value size", out_value_size, expected_value_size) != 0 ||
    expect_bytes(
      "out param value bytes",
      (const unsigned char *)out_param.value,
      expected_value,
      expected_value_size) != 0) {
    return 1;
  }

  return 0;
}

static int list_contains_name(
  const char * names,
  size_t names_size,
  const char * expected_name)
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

static int expect_list_has_names(
  const char * names,
  size_t names_size,
  const char * const * expected_names,
  size_t expected_count)
{
  for (size_t i = 0u; i < expected_count; ++i) {
    if (!list_contains_name(names, names_size, expected_names[i])) {
      (void)fprintf(stderr, "FAIL list missing name %s\n", expected_names[i]);
      return 1;
    }
  }

  return 0;
}

static int test_null_default_runtime_params_unsupported(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t value = 1u;
  grcl_param_record_t input = param_record("alpha", GRCL_PARAM_TYPE_BOOL, &value, sizeof(value));
  grcl_param_record_t out_param = {0};
  unsigned char value_buffer[8u] = {0};
  size_t out_value_size = 0u;
  char name_buffer[8u] = {0};
  size_t names_size = 0u;
  size_t param_count = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "null/default set unsupported before start",
      grcl_runtime_param_set(runtime, &input),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null/default get unsupported before start",
      grcl_runtime_param_get(
        runtime,
        "alpha",
        &out_param,
        value_buffer,
        sizeof(value_buffer),
        &out_value_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null/default list unsupported before start",
      grcl_runtime_param_list(
        runtime,
        name_buffer,
        sizeof(name_buffer),
        &names_size,
        &param_count),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("start null/default runtime", grcl_runtime_start(runtime), GRCL_OK) != 0 ||
    expect_result(
      "null/default set unsupported while started",
      grcl_runtime_param_set(runtime, &input),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("stop null/default runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0 ||
    expect_result(
      "null/default list unsupported after stop",
      grcl_runtime_param_list(
        runtime,
        name_buffer,
        sizeof(name_buffer),
        &names_size,
        &param_count),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null/default runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_set_get_all_types_and_copy_semantics_before_start(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t bool_value = 1u;
  const int64_t int_value = -42;
  const uint64_t uint_value = 42u;
  const double float_value = 3.25;
  const char string_value[] = "hello-params";
  char mutable_name[] = "bytes.mutable";
  unsigned char mutable_value[] = {9u, 8u, 7u, 6u};
  const unsigned char expected_bytes[] = {9u, 8u, 7u, 6u};

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set bool",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.bool",
          GRCL_PARAM_TYPE_BOOL,
          &bool_value,
          sizeof(bool_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "set int64",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.int64",
          GRCL_PARAM_TYPE_INT64,
          &int_value,
          sizeof(int_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "set uint64",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.uint64",
          GRCL_PARAM_TYPE_UINT64,
          &uint_value,
          sizeof(uint_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "set float64",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.float64",
          GRCL_PARAM_TYPE_FLOAT64,
          &float_value,
          sizeof(float_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "set string",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.string",
          GRCL_PARAM_TYPE_STRING,
          string_value,
          sizeof(string_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "set bytes",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          mutable_name,
          GRCL_PARAM_TYPE_BYTES,
          mutable_value,
          sizeof(mutable_value)}),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  mutable_name[0] = 'X';
  mutable_value[0] = 0u;
  mutable_value[1] = 0u;
  mutable_value[2] = 0u;
  mutable_value[3] = 0u;

  if (expect_param_get_success(
      runtime,
      "param.bool",
      GRCL_PARAM_TYPE_BOOL,
      (const unsigned char *)&bool_value,
      sizeof(bool_value)) != 0 ||
    expect_param_get_success(
      runtime,
      "param.int64",
      GRCL_PARAM_TYPE_INT64,
      (const unsigned char *)&int_value,
      sizeof(int_value)) != 0 ||
    expect_param_get_success(
      runtime,
      "param.uint64",
      GRCL_PARAM_TYPE_UINT64,
      (const unsigned char *)&uint_value,
      sizeof(uint_value)) != 0 ||
    expect_param_get_success(
      runtime,
      "param.float64",
      GRCL_PARAM_TYPE_FLOAT64,
      (const unsigned char *)&float_value,
      sizeof(float_value)) != 0 ||
    expect_param_get_success(
      runtime,
      "param.string",
      GRCL_PARAM_TYPE_STRING,
      (const unsigned char *)string_value,
      sizeof(string_value)) != 0 ||
    expect_param_get_success(
      runtime,
      "bytes.mutable",
      GRCL_PARAM_TYPE_BYTES,
      expected_bytes,
      sizeof(expected_bytes)) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_overwrite_and_missing_param(void)
{
  grcl_runtime_t * runtime = NULL;
  int64_t first_value = 11;
  const char second_value[] = "updated";
  grcl_param_record_t out_param = {0};
  unsigned char value_buffer[32u] = {0};
  size_t out_value_size = 0u;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set original value",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.alpha",
          GRCL_PARAM_TYPE_INT64,
          &first_value,
          sizeof(first_value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "overwrite value",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.alpha",
          GRCL_PARAM_TYPE_STRING,
          second_value,
          sizeof(second_value)}),
      GRCL_OK) != 0 ||
    expect_param_get_success(
      runtime,
      "param.alpha",
      GRCL_PARAM_TYPE_STRING,
      (const unsigned char *)second_value,
      sizeof(second_value)) != 0 ||
    expect_result(
      "missing name",
      grcl_runtime_param_get(
        runtime,
        "param.missing",
        &out_param,
        value_buffer,
        sizeof(value_buffer),
        &out_value_size),
      GRCL_ERROR_NOT_FOUND) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_invalid_arguments(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  grcl_param_record_t out_param = {0};
  size_t out_value_size = 0u;
  char names[16u] = {0};
  size_t names_size = 0u;
  size_t param_count = 0u;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set empty name",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "",
          GRCL_PARAM_TYPE_BOOL,
          &one,
          sizeof(one)}),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "set unknown type",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.invalid.type",
          GRCL_PARAM_TYPE_UNKNOWN,
          &one,
          sizeof(one)}),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "set null value with size",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.invalid.value",
          GRCL_PARAM_TYPE_BOOL,
          NULL,
          sizeof(one)}),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "get null out param",
      grcl_runtime_param_get(
        runtime,
        "param.any",
        NULL,
        names,
        sizeof(names),
        &out_value_size),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "get null out size",
      grcl_runtime_param_get(
        runtime,
        "param.any",
        &out_param,
        names,
        sizeof(names),
        NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "get null buffer with capacity",
      grcl_runtime_param_get(
        runtime,
        "param.any",
        &out_param,
        NULL,
        1u,
        &out_value_size),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "list null size output",
      grcl_runtime_param_list(runtime, names, sizeof(names), NULL, &param_count),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "list null count output",
      grcl_runtime_param_list(runtime, names, sizeof(names), &names_size, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "list null names with capacity",
      grcl_runtime_param_list(runtime, NULL, 1u, &names_size, &param_count),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_small_get_buffer_then_success(void)
{
  grcl_runtime_t * runtime = NULL;
  const char value[] = "buffer-check";
  grcl_param_record_t out_param = {0};
  char small_buffer[4u] = {0};
  char full_buffer[32u] = {0};
  size_t out_value_size = 0u;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set string param",
      grcl_runtime_param_set(
        runtime,
        &(grcl_param_record_t){
          sizeof(grcl_param_record_t),
          GRCL_C_ABI_VERSION_CURRENT,
          "param.buffer",
          GRCL_PARAM_TYPE_STRING,
          value,
          sizeof(value)}),
      GRCL_OK) != 0 ||
    expect_result(
      "small get buffer",
      grcl_runtime_param_get(
        runtime,
        "param.buffer",
        &out_param,
        small_buffer,
        sizeof(small_buffer),
        &out_value_size),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small get required size", out_value_size, sizeof(value)) != 0 ||
    expect_result(
      "full get buffer",
      grcl_runtime_param_get(
        runtime,
        "param.buffer",
        &out_param,
        full_buffer,
        sizeof(full_buffer),
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_ptr("full get value pointer", out_param.value, full_buffer) != 0 ||
    expect_bytes(
      "full get value bytes",
      (const unsigned char *)full_buffer,
      (const unsigned char *)value,
      sizeof(value)) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_list_small_buffer_then_success(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  char small_names[8u] = {0};
  char full_names[64u] = {0};
  size_t names_size = 0u;
  size_t param_count = 0u;
  const char * expected_names[] = {"alpha", "beta", "gamma"};

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 3u; ++i) {
    if (expect_result(
        "set list param",
        grcl_runtime_param_set(
          runtime,
          &(grcl_param_record_t){
            sizeof(grcl_param_record_t),
            GRCL_C_ABI_VERSION_CURRENT,
            expected_names[i],
            GRCL_PARAM_TYPE_BOOL,
            &one,
            sizeof(one)}),
        GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "list small buffer",
      grcl_runtime_param_list(
        runtime,
        small_names,
        sizeof(small_names),
        &names_size,
        &param_count),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small list count", param_count, 3u) != 0 ||
    expect_result(
      "list full buffer",
      grcl_runtime_param_list(
        runtime,
        full_names,
        sizeof(full_names),
        &names_size,
        &param_count),
      GRCL_OK) != 0 ||
    expect_size("full list count", param_count, 3u) != 0 ||
    expect_list_has_names(full_names, names_size, expected_names, 3u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_parameter_table_saturation(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  char name[32u] = {0};
  grcl_param_record_t record;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 8u; ++i) {
    (void)snprintf(name, sizeof(name), "param.slot.%zu", i);
    record = param_record(name, GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
    if (expect_result(
        "fill parameter slots",
        grcl_runtime_param_set(runtime, &record),
        GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record("param.slot.overflow", GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
  if (expect_result(
      "parameter table saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_name_storage_saturation(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  grcl_param_record_t record;
  const char * long_names[] = {
    "param_name_slot_001",
    "param_name_slot_002",
    "param_name_slot_003",
    "param_name_slot_004",
    "param_name_slot_005",
    "param_name_slot_006",
    "param_name_slot_007"
  };

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 6u; ++i) {
    record = param_record(long_names[i], GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
    if (expect_result(
        "fill name storage",
        grcl_runtime_param_set(runtime, &record),
        GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record(long_names[6], GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
  if (expect_result(
      "name storage saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_value_storage_saturation(void)
{
  grcl_runtime_t * runtime = NULL;
  unsigned char value[170u];
  char name[32u] = {0};
  grcl_param_record_t record;

  memset(value, 0xAB, sizeof(value));

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 6u; ++i) {
    (void)snprintf(name, sizeof(name), "param.value.%zu", i);
    value[0] = (unsigned char)(0xA0u + i);
    record = param_record(name, GRCL_PARAM_TYPE_BYTES, value, sizeof(value));
    if (expect_result(
        "fill value storage",
        grcl_runtime_param_set(runtime, &record),
        GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record("param.value.overflow", GRCL_PARAM_TYPE_BYTES, value, sizeof(value));
  if (expect_result(
      "value storage saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_params_work_before_start_while_started_after_stop(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t before_start_value = 1u;
  const uint8_t while_started_value = 0u;
  const char after_stop_value[] = "stopped";
  char names[128u] = {0};
  size_t names_size = 0u;
  size_t param_count = 0u;
  grcl_param_record_t record;
  const char * expected_names[] = {"param.before", "param.started", "param.stopped"};

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  record = param_record(
    "param.before",
    GRCL_PARAM_TYPE_BOOL,
    &before_start_value,
    sizeof(before_start_value));
  if (expect_result(
      "set before start",
      grcl_runtime_param_set(runtime, &record),
      GRCL_OK) != 0 ||
    expect_param_get_success(
      runtime,
      "param.before",
      GRCL_PARAM_TYPE_BOOL,
      &before_start_value,
      sizeof(before_start_value)) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  record = param_record(
    "param.started",
    GRCL_PARAM_TYPE_BOOL,
    &while_started_value,
    sizeof(while_started_value));
  if (expect_result(
      "set while started",
      grcl_runtime_param_set(runtime, &record),
      GRCL_OK) != 0 ||
    expect_param_get_success(
      runtime,
      "param.started",
      GRCL_PARAM_TYPE_BOOL,
      &while_started_value,
      sizeof(while_started_value)) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  record = param_record(
    "param.stopped",
    GRCL_PARAM_TYPE_STRING,
    after_stop_value,
    sizeof(after_stop_value));
  if (expect_result(
      "set after stop",
      grcl_runtime_param_set(runtime, &record),
      GRCL_OK) != 0 ||
    expect_param_get_success(
      runtime,
      "param.stopped",
      GRCL_PARAM_TYPE_STRING,
      (const unsigned char *)after_stop_value,
      sizeof(after_stop_value)) != 0 ||
    expect_result(
      "list after stop",
      grcl_runtime_param_list(runtime, names, sizeof(names), &names_size, &param_count),
      GRCL_OK) != 0 ||
    expect_size("list count after stop", param_count, 3u) != 0 ||
    expect_list_has_names(names, names_size, expected_names, 3u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  int failures = 0;

  failures += test_null_default_runtime_params_unsupported();
  failures += test_set_get_all_types_and_copy_semantics_before_start();
  failures += test_overwrite_and_missing_param();
  failures += test_invalid_arguments();
  failures += test_small_get_buffer_then_success();
  failures += test_list_small_buffer_then_success();
  failures += test_parameter_table_saturation();
  failures += test_name_storage_saturation();
  failures += test_value_storage_saturation();
  failures += test_params_work_before_start_while_started_after_stop();

  return failures == 0 ? 0 : 1;
}
