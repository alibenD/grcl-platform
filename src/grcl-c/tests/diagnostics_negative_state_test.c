#include <stddef.h>
#include <stdalign.h>

#include <grcl/c/backend.h>
#include <grcl/c/capability.h>
#include <grcl/c/diagnostics.h>
#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
#include <grcl/c/version.h>

static int expect_result(
  const char * label,
  grcl_result_t actual,
  grcl_result_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)label;
  return 1;
}

static int expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)label;
  return 1;
}

static int expect_u32(const char * label, uint32_t actual, uint32_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)label;
  return 1;
}

typedef struct runtime_storage_fixture {
  union {
    max_align_t alignment;
    unsigned char bytes[512u];
  } runtime_object;
  grcl_storage_region_t runtime_region;
  grcl_storage_t storage;
} runtime_storage_fixture_t;

static grcl_storage_t minimal_bounded_storage_without_regions(void)
{
  grcl_storage_t storage;
  storage.struct_size = sizeof(storage);
  storage.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  storage.flags = GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES;
  storage.regions = NULL;
  storage.region_count = 0u;
  storage.runtime_object_bytes = 0u;
  storage.node_table_capacity = 0u;
  storage.endpoint_table_capacity = 0u;
  storage.session_table_capacity = 0u;
  storage.channel_table_capacity = 0u;
  storage.message_buffer_bytes = 0u;
  storage.graph_cache_bytes = 0u;
  storage.diagnostics_buffer_bytes = 0u;
  storage.executor_state_bytes = 0u;
  return storage;
}

static void init_runtime_storage_fixture(runtime_storage_fixture_t * fixture)
{
  fixture->runtime_region.struct_size = sizeof(fixture->runtime_region);
  fixture->runtime_region.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  fixture->runtime_region.kind = GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT;
  fixture->runtime_region.data = fixture->runtime_object.bytes;
  fixture->runtime_region.bytes = sizeof(fixture->runtime_object.bytes);
  fixture->runtime_region.alignment = alignof(max_align_t);
  fixture->runtime_region.flags = GRCL_STORAGE_FLAG_NONE;

  fixture->storage = minimal_bounded_storage_without_regions();
  fixture->storage.regions = &fixture->runtime_region;
  fixture->storage.region_count = 1u;
  fixture->storage.runtime_object_bytes = sizeof(fixture->runtime_object.bytes);
}

static grcl_runtime_capability_request_t incompatible_request(void)
{
  grcl_runtime_capability_request_t request = {0};
  request.struct_size = sizeof(request);
  request.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  request.scope = GRCL_CAPABILITY_SCOPE_SESSION;
  request.required_profile_id = 1u;
  request.required_runtime_class_id = GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST;
  request.min_grcl_protocol_major = 0u;
  request.min_grcl_protocol_minor = 1u;
  request.max_grcl_protocol_major = 0u;
  request.max_grcl_protocol_minor = 1u;
  request.min_capability_schema_version = 1u;
  request.required_graph_projection_modes = GRCL_GRAPH_PROJECTION_MODE_FLAG_FULL;
  return request;
}

static int test_null_runtime_diagnostics_is_invalid_argument(void)
{
  size_t count = 0u;

  return expect_result(
    "null runtime diagnostics",
    grcl_runtime_get_diagnostics(NULL, NULL, 0u, &count),
    GRCL_ERROR_INVALID_ARGUMENT);
}

static int test_null_record_buffer_with_capacity_is_invalid_argument(void)
{
  grcl_runtime_t * runtime = NULL;
  size_t count = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "null records with capacity",
      grcl_runtime_get_diagnostics(runtime, NULL, 1u, &count),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_count_query_accepts_null_record_buffer(void)
{
  grcl_runtime_t * runtime = NULL;
  size_t count = 99u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "diagnostics count query",
      grcl_runtime_get_diagnostics(runtime, NULL, 0u, &count),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_size("diagnostics count", count, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_repeated_start_records_bad_state_diagnostic(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_diagnostic_record_t records[1u];
  size_t count = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_result(
      "repeated start",
      grcl_runtime_start(runtime),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "diagnostics after repeated start",
      grcl_runtime_get_diagnostics(runtime, records, 1u, &count),
      GRCL_OK) != 0 ||
    expect_size("diagnostic count", count, 1u) != 0 ||
    expect_u32(
      "diagnostic code",
      (uint32_t)records[0u].code,
      (uint32_t)GRCL_DIAGNOSTIC_CODE_BAD_STATE) != 0 ||
    expect_u32(
      "diagnostic category",
      (uint32_t)records[0u].category,
      (uint32_t)GRCL_DIAGNOSTIC_CATEGORY_CALLER_ERROR) != 0 ||
    expect_u32(
      "diagnostic scope",
      (uint32_t)records[0u].scope,
      (uint32_t)GRCL_DIAGNOSTIC_SCOPE_RUNTIME) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (grcl_runtime_stop(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_repeated_stop_records_bad_state_diagnostic(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_diagnostic_record_t records[1u];
  size_t count = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_runtime_start(runtime) != GRCL_OK ||
    grcl_runtime_stop(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_result(
      "repeated stop",
      grcl_runtime_stop(runtime),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "diagnostics after repeated stop",
      grcl_runtime_get_diagnostics(runtime, records, 1u, &count),
      GRCL_OK) != 0 ||
    expect_size("diagnostic count", count, 1u) != 0 ||
    expect_u32(
      "diagnostic code",
      (uint32_t)records[0u].code,
      (uint32_t)GRCL_DIAGNOSTIC_CODE_BAD_STATE) != 0 ||
    expect_u32(
      "diagnostic category",
      (uint32_t)records[0u].category,
      (uint32_t)GRCL_DIAGNOSTIC_CATEGORY_CALLER_ERROR) != 0 ||
    expect_u32(
      "diagnostic scope",
      (uint32_t)records[0u].scope,
      (uint32_t)GRCL_DIAGNOSTIC_SCOPE_RUNTIME) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_incompatible_capability_request_is_rejected(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_request_t request = incompatible_request();
  grcl_capability_negotiation_result_t result = {0};

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "incompatible negotiation",
      grcl_runtime_negotiate_capabilities(runtime, &request, &result),
      GRCL_OK) != 0 ||
    expect_u32(
      "rejected incompatible status",
      (uint32_t)result.status,
      (uint32_t)GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_missing_bounded_storage_is_capacity_exceeded(void)
{
  runtime_storage_fixture_t fixture;
  grcl_storage_t storage;
  grcl_runtime_t * runtime = NULL;

  init_runtime_storage_fixture(&fixture);
  storage = fixture.storage;
  storage.flags = GRCL_STORAGE_FLAG_NONE;

  if (expect_result(
      "init unbounded storage",
      grcl_runtime_init_with_storage(NULL, &storage, &runtime),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    return 1;
  }

  return runtime == NULL ? 0 : 1;
}

int main(void)
{
  int failures = 0;

  failures += test_null_runtime_diagnostics_is_invalid_argument();
  failures += test_null_record_buffer_with_capacity_is_invalid_argument();
  failures += test_count_query_accepts_null_record_buffer();
  failures += test_repeated_start_records_bad_state_diagnostic();
  failures += test_repeated_stop_records_bad_state_diagnostic();
  failures += test_incompatible_capability_request_is_rejected();
  failures += test_missing_bounded_storage_is_capacity_exceeded();

  return failures == 0 ? 0 : 1;
}
