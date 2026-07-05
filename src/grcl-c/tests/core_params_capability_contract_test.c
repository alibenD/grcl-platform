#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/capability.h>
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

static int expect_flag_set(const char * label, uint32_t actual, uint32_t flag)
{
  if ((actual & flag) == flag) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: flag 0x%x missing from 0x%x\n", label, flag, actual);
  return 1;
}

static int expect_flag_unset(const char * label, uint32_t actual, uint32_t flag)
{
  if ((actual & flag) == 0u) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: flag 0x%x unexpectedly set in 0x%x\n", label, flag, actual);
  return 1;
}

static int expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (size == 0u || memcmp(actual, expected, size) == 0) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: bytes differ\n", label);
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

static grcl_runtime_options_t inprocess_runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "m4-e-params-capability";
  options.profile_name = "native-inprocess";
  return options;
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

#define param_record(name, type, value, value_size) \
  ((grcl_param_record_t){ \
    sizeof(grcl_param_record_t), \
    GRCL_C_ABI_VERSION_CURRENT, \
    name, \
    type, \
    value, \
    value_size \
  })

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

static int test_zero_size_values_are_supported(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t placeholder = 0u;
  grcl_param_record_t out_param = {0};
  size_t out_value_size = 99u;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set zero-size string",
      grcl_runtime_param_set(
        runtime,
        &param_record("zero.string", GRCL_PARAM_TYPE_STRING, &placeholder, 0u)),
      GRCL_OK) != 0 ||
    expect_result(
      "get zero-size string",
      grcl_runtime_param_get(
        runtime,
        "zero.string",
        &out_param,
        NULL,
        0u,
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_size("zero-size string value size", out_value_size, 0u) != 0 ||
    expect_string("zero-size string name", out_param.name, "zero.string") != 0 ||
    expect_u32(
      "zero-size string type",
      (uint32_t)out_param.type,
      (uint32_t)GRCL_PARAM_TYPE_STRING) != 0 ||
    expect_size("zero-size string record size", out_param.value_size, 0u) != 0 ||
    expect_result(
      "set zero-size bytes",
      grcl_runtime_param_set(
        runtime,
        &param_record("zero.bytes", GRCL_PARAM_TYPE_BYTES, &placeholder, 0u)),
      GRCL_OK) != 0 ||
    expect_result(
      "get zero-size bytes",
      grcl_runtime_param_get(
        runtime,
        "zero.bytes",
        &out_param,
        NULL,
        0u,
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_size("zero-size bytes value size", out_value_size, 0u) != 0 ||
    expect_string("zero-size bytes name", out_param.name, "zero.bytes") != 0 ||
    expect_u32(
      "zero-size bytes type",
      (uint32_t)out_param.type,
      (uint32_t)GRCL_PARAM_TYPE_BYTES) != 0 ||
    expect_size("zero-size bytes record size", out_param.value_size, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_invalid_param_names_and_values_are_rejected(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  grcl_param_record_t out_param = {0};
  size_t out_value_size = 0u;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set null name",
      grcl_runtime_param_set(
        runtime,
        &param_record(NULL, GRCL_PARAM_TYPE_BOOL, &one, sizeof(one))),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "set empty name",
      grcl_runtime_param_set(
        runtime,
        &param_record("", GRCL_PARAM_TYPE_BOOL, &one, sizeof(one))),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "get null name",
      grcl_runtime_param_get(
        runtime,
        NULL,
        &out_param,
        &out_value_size,
        sizeof(out_value_size),
        &out_value_size),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "get empty name",
      grcl_runtime_param_get(
        runtime,
        "",
        &out_param,
        &out_value_size,
        sizeof(out_value_size),
        &out_value_size),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "set unknown type",
      grcl_runtime_param_set(
        runtime,
        &param_record("bad.type", GRCL_PARAM_TYPE_UNKNOWN, &one, sizeof(one))),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "set null value with size",
      grcl_runtime_param_set(
        runtime,
        &param_record("bad.value", GRCL_PARAM_TYPE_BOOL, NULL, sizeof(one))),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_overwrite_compacts_value_storage(void)
{
  grcl_runtime_t * runtime = NULL;
  unsigned char large_a[500u];
  unsigned char large_b[500u];
  unsigned char large_c[500u];
  const unsigned char small_value[] = {0x5Au};
  grcl_param_record_t out_param = {0};
  unsigned char read_back[512u] = {0};
  size_t out_value_size = 0u;

  memset(large_a, 0xA1, sizeof(large_a));
  memset(large_b, 0xB2, sizeof(large_b));
  memset(large_c, 0xC3, sizeof(large_c));

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "set large a",
      grcl_runtime_param_set(
        runtime,
        &param_record("a", GRCL_PARAM_TYPE_BYTES, large_a, sizeof(large_a))),
      GRCL_OK) != 0 ||
    expect_result(
      "set large b",
      grcl_runtime_param_set(
        runtime,
        &param_record("b", GRCL_PARAM_TYPE_BYTES, large_b, sizeof(large_b))),
      GRCL_OK) != 0 ||
    expect_result(
      "overwrite a with small value",
      grcl_runtime_param_set(
        runtime,
        &param_record("a", GRCL_PARAM_TYPE_BYTES, small_value, sizeof(small_value))),
      GRCL_OK) != 0 ||
    expect_result(
      "set large c after compaction",
      grcl_runtime_param_set(
        runtime,
        &param_record("c", GRCL_PARAM_TYPE_BYTES, large_c, sizeof(large_c))),
      GRCL_OK) != 0 ||
    expect_result(
      "get overwritten a",
      grcl_runtime_param_get(
        runtime,
        "a",
        &out_param,
        read_back,
        sizeof(read_back),
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_size("overwrite a size", out_value_size, sizeof(small_value)) != 0 ||
    expect_bytes("overwrite a value", read_back, small_value, sizeof(small_value)) != 0 ||
    expect_result(
      "get added c",
      grcl_runtime_param_get(
        runtime,
        "c",
        &out_param,
        read_back,
        sizeof(read_back),
        &out_value_size),
      GRCL_OK) != 0 ||
    expect_size("added c size", out_value_size, sizeof(large_c)) != 0 ||
    expect_bytes("added c value", read_back, large_c, sizeof(large_c)) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_list_reports_exact_size_and_is_non_destructive_on_capacity_error(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  size_t names_size = 0u;
  size_t param_count = 0u;
  char exact_names[64u] = {0};
  char small_names[64u] = {0};
  char verify_names[64u] = {0};
  const char * expected_names[] = {"alpha", "beta", "gamma"};

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 3u; ++i) {
    if (expect_result(
        "set list entry",
        grcl_runtime_param_set(
          runtime,
          &param_record(expected_names[i], GRCL_PARAM_TYPE_BOOL, &one, sizeof(one))),
        GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "list size query",
      grcl_runtime_param_list(runtime, NULL, 0u, &names_size, &param_count),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("list size query names size", names_size, 16u) != 0 ||
    expect_size("list size query param count", param_count, 3u) != 0 ||
    expect_result(
      "list one byte small",
      grcl_runtime_param_list(runtime, small_names, names_size - 1u, &names_size, &param_count),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("list small names size", names_size, 16u) != 0 ||
    expect_size("list small param count", param_count, 3u) != 0 ||
    expect_result(
      "list exact size",
      grcl_runtime_param_list(runtime, exact_names, names_size, &names_size, &param_count),
      GRCL_OK) != 0 ||
    expect_size("list exact names size", names_size, 16u) != 0 ||
    expect_size("list exact param count", param_count, 3u) != 0 ||
    expect_result(
      "list again after small-buffer failure",
      grcl_runtime_param_list(
        runtime,
        verify_names,
        sizeof(verify_names),
        &names_size,
        &param_count),
      GRCL_OK) != 0 ||
    expect_size("list verify names size", names_size, 16u) != 0 ||
    expect_size("list verify param count", param_count, 3u) != 0 ||
    !list_contains_name(exact_names, names_size, "alpha") ||
    !list_contains_name(exact_names, names_size, "beta") ||
    !list_contains_name(exact_names, names_size, "gamma") ||
    !list_contains_name(verify_names, names_size, "alpha") ||
    !list_contains_name(verify_names, names_size, "beta") ||
    !list_contains_name(verify_names, names_size, "gamma")) {
    if (!list_contains_name(exact_names, names_size, "alpha") ||
      !list_contains_name(exact_names, names_size, "beta") ||
      !list_contains_name(exact_names, names_size, "gamma") ||
      !list_contains_name(verify_names, names_size, "alpha") ||
      !list_contains_name(verify_names, names_size, "beta") ||
      !list_contains_name(verify_names, names_size, "gamma")) {
      (void)fprintf(stderr, "FAIL list names missing after verification\n");
    }
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_param_capacity_boundaries(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  unsigned char value[170u];
  grcl_param_record_t record;
  char name[32u] = {0};
  const char * long_names[] = {
    "param_name_slot_001",
    "param_name_slot_002",
    "param_name_slot_003",
    "param_name_slot_004",
    "param_name_slot_005",
    "param_name_slot_006",
    "param_name_slot_007"
  };

  memset(value, 0xAB, sizeof(value));

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 8u; ++i) {
    (void)snprintf(name, sizeof(name), "slot.%zu", i);
    record = param_record(name, GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
    if (expect_result("fill param table", grcl_runtime_param_set(runtime, &record), GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record("slot.overflow", GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
  if (expect_result(
      "param table saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("destroy table runtime", grcl_runtime_destroy(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 6u; ++i) {
    record = param_record(long_names[i], GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
    if (expect_result("fill name storage", grcl_runtime_param_set(runtime, &record), GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record(long_names[6], GRCL_PARAM_TYPE_BOOL, &one, sizeof(one));
  if (expect_result(
      "name storage saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("destroy name runtime", grcl_runtime_destroy(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  for (size_t i = 0u; i < 6u; ++i) {
    (void)snprintf(name, sizeof(name), "value.%zu", i);
    value[0] = (unsigned char)(0xA0u + i);
    record = param_record(name, GRCL_PARAM_TYPE_BYTES, value, sizeof(value));
    if (expect_result("fill value storage", grcl_runtime_param_set(runtime, &record), GRCL_OK) != 0) {
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  record = param_record("value.overflow", GRCL_PARAM_TYPE_BYTES, value, sizeof(value));
  if (expect_result(
      "value storage saturation",
      grcl_runtime_param_set(runtime, &record),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy value runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_null_default_backend_params_are_unsupported(void)
{
  grcl_runtime_t * runtime = NULL;
  const uint8_t one = 1u;
  grcl_param_record_t out_param = {0};
  size_t out_value_size = 0u;
  size_t names_size = 0u;
  size_t param_count = 0u;

  if (expect_result("create null runtime", grcl_runtime_create(NULL, &runtime), GRCL_OK) != 0) {
    return 1;
  }

  if (expect_result(
      "null backend set unsupported",
      grcl_runtime_param_set(runtime, &param_record("alpha", GRCL_PARAM_TYPE_BOOL, &one, sizeof(one))),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null backend get unsupported",
      grcl_runtime_param_get(
        runtime,
        "alpha",
        &out_param,
        &out_value_size,
        sizeof(out_value_size),
        &out_value_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null backend list unsupported",
      grcl_runtime_param_list(runtime, NULL, 0u, &names_size, &param_count),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_native_capability_reports_only_m3_local_behavior(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_record_t record = {0};
  const uint32_t required_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY |
    GRCL_CAPABILITY_SUMMARY_FLAG_POLL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_SEQUENTIAL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS |
    GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS;

  if (create_inprocess_runtime(&runtime) != 0) {
    return 1;
  }

  if (expect_result(
      "native capability query",
      grcl_runtime_get_capabilities(runtime, &record),
      GRCL_OK) != 0 ||
    expect_flag_set("native required flags", record.summary_flags, required_flags) != 0 ||
    expect_flag_unset(
      "native trigger executor absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_TRIGGER_EXECUTOR) != 0 ||
    expect_flag_unset(
      "native multithread executor absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_MULTI_THREAD_EXECUTOR) != 0 ||
    expect_flag_unset(
      "native security family absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_SECURITY_FAMILY_PRESENT) != 0 ||
    expect_flag_unset(
      "native dynamic endpoint delta absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_DYNAMIC_ENDPOINT_DELTA) != 0 ||
    expect_flag_unset(
      "native static manifest absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_MANIFEST) != 0 ||
    expect_size("native max payload", record.max_payload_bytes, 1024u) != 0 ||
    expect_size("native max entities", record.max_entities, 32u) != 0 ||
    expect_size("native max nodes", record.max_nodes, 8u) != 0 ||
    expect_size("native max endpoints", record.max_endpoints, 32u) != 0 ||
    expect_size("native max publishers", record.max_publishers, 16u) != 0 ||
    expect_size("native max subscriptions", record.max_subscriptions, 16u) != 0 ||
    expect_size("native max services", record.max_services, 16u) != 0 ||
    expect_size("native max clients", record.max_clients, 16u) != 0 ||
    expect_size("native graph cache bytes", record.graph_cache_bytes, 0u) != 0 ||
    expect_size("native transport descriptor count", record.transport_descriptor_count, 0u) != 0 ||
    expect_size("native transport descriptor capacity", record.transport_descriptor_capacity, 0u) != 0 ||
    expect_size("native qos descriptor count", record.qos_descriptor_count, 0u) != 0 ||
    expect_size("native qos descriptor capacity", record.qos_descriptor_capacity, 0u) != 0 ||
    expect_size("native encoding descriptor count", record.encoding_descriptor_count, 0u) != 0 ||
    expect_size("native encoding descriptor capacity", record.encoding_descriptor_capacity, 0u) != 0 ||
    expect_size("native diagnostics descriptor count", record.diagnostics_descriptor_count, 0u) != 0 ||
    expect_size("native diagnostics descriptor capacity", record.diagnostics_descriptor_capacity, 0u) != 0 ||
    expect_size("native security descriptor count", record.security_descriptor_count, 0u) != 0 ||
    expect_size("native security descriptor capacity", record.security_descriptor_capacity, 0u) != 0 ||
    expect_size("native max parameters", record.max_parameters, 8u) != 0 ||
    expect_size("native name buffer bytes", record.parameter_name_buffer_bytes, 128u) != 0 ||
    expect_size("native value buffer bytes", record.parameter_value_buffer_bytes, 1024u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy native runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_null_capability_does_not_claim_local_routing_or_params(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_record_t record = {0};

  if (expect_result("create null runtime", grcl_runtime_create(NULL, &runtime), GRCL_OK) != 0) {
    return 1;
  }

  if (expect_result(
      "null capability query",
      grcl_runtime_get_capabilities(runtime, &record),
      GRCL_OK) != 0 ||
    expect_flag_unset(
      "null local params absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS) != 0 ||
    expect_flag_unset(
      "null poll executor absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_POLL_EXECUTOR) != 0 ||
    expect_flag_unset(
      "null sequential executor absent",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_SEQUENTIAL_EXECUTOR) != 0 ||
    expect_size("null max payload", record.max_payload_bytes, 0u) != 0 ||
    expect_size("null max publishers", record.max_publishers, 0u) != 0 ||
    expect_size("null max subscriptions", record.max_subscriptions, 0u) != 0 ||
    expect_size("null max services", record.max_services, 0u) != 0 ||
    expect_size("null max clients", record.max_clients, 0u) != 0 ||
    expect_size("null executor state bytes", record.executor_state_bytes, 0u) != 0 ||
    expect_size("null transport descriptor count", record.transport_descriptor_count, 0u) != 0 ||
    expect_size("null graph cache bytes", record.graph_cache_bytes, 0u) != 0 ||
    expect_size("null security descriptor count", record.security_descriptor_count, 0u) != 0 ||
    expect_size("null security descriptor capacity", record.security_descriptor_capacity, 0u) != 0 ||
    expect_size("null max parameters", record.max_parameters, 0u) != 0 ||
    expect_size("null name buffer bytes", record.parameter_name_buffer_bytes, 0u) != 0 ||
    expect_size("null value buffer bytes", record.parameter_value_buffer_bytes, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  int failures = 0;

  failures += test_zero_size_values_are_supported();
  failures += test_invalid_param_names_and_values_are_rejected();
  failures += test_overwrite_compacts_value_storage();
  failures += test_list_reports_exact_size_and_is_non_destructive_on_capacity_error();
  failures += test_param_capacity_boundaries();
  failures += test_null_default_backend_params_are_unsupported();
  failures += test_native_capability_reports_only_m3_local_behavior();
  failures += test_null_capability_does_not_claim_local_routing_or_params();

  return failures == 0 ? 0 : 1;
}
