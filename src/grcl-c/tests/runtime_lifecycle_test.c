#include <stddef.h>
#include <stdalign.h>

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

static int expect_non_null(const char * label, const void * value)
{
  if (value != NULL) {
    return 0;
  }

  (void)label;
  return 1;
}

static int expect_pointer(
  const char * label,
  const void * actual,
  const void * expected)
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

static int test_create_returns_runtime(void)
{
  grcl_runtime_t * runtime = NULL;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(NULL, &runtime),
      GRCL_OK) != 0) {
    return 1;
  }
  if (expect_non_null("runtime handle", runtime) != 0) {
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_start_stop_destroy_order(void)
{
  grcl_runtime_t * runtime = NULL;
  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_null_start_is_invalid_argument(void)
{
  return expect_result(
    "start null runtime",
    grcl_runtime_start(NULL),
    GRCL_ERROR_INVALID_ARGUMENT);
}

static int test_repeated_start_is_bad_state(void)
{
  grcl_runtime_t * runtime = NULL;
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

  if (grcl_runtime_stop(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_repeated_stop_is_bad_state(void)
{
  grcl_runtime_t * runtime = NULL;
  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (grcl_runtime_stop(runtime) != GRCL_OK) {
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

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_init_with_bounded_storage_succeeds(void)
{
  runtime_storage_fixture_t fixture;
  grcl_runtime_t * runtime = NULL;
  init_runtime_storage_fixture(&fixture);

  if (expect_result(
      "init bounded storage",
      grcl_runtime_init_with_storage(NULL, &fixture.storage, &runtime),
      GRCL_OK) != 0) {
    return 1;
  }
  if (expect_non_null("runtime handle", runtime) != 0) {
    return 1;
  }
  if (expect_pointer(
      "runtime handle storage placement",
      runtime,
      fixture.runtime_object.bytes) != 0) {
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_init_without_bounded_storage_fails(void)
{
  runtime_storage_fixture_t fixture;
  init_runtime_storage_fixture(&fixture);
  grcl_storage_t storage = fixture.storage;
  storage.flags = GRCL_STORAGE_FLAG_NONE;

  grcl_runtime_t * runtime = NULL;
  if (expect_result(
      "init null storage",
      grcl_runtime_init_with_storage(NULL, NULL, &runtime),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    return 1;
  }
  if (runtime != NULL) {
    return 1;
  }

  return expect_result(
    "init unbounded storage",
    grcl_runtime_init_with_storage(NULL, &storage, &runtime),
    GRCL_ERROR_CAPACITY_EXCEEDED);
}

static int test_init_with_bounded_storage_missing_runtime_region_fails(void)
{
  grcl_storage_t storage = minimal_bounded_storage_without_regions();
  grcl_runtime_t * runtime = NULL;

  if (expect_result(
      "init bounded storage missing runtime region",
      grcl_runtime_init_with_storage(NULL, &storage, &runtime),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    return 1;
  }

  return runtime == NULL ? 0 : 1;
}

int main(void)
{
  int failures = 0;

  failures += test_create_returns_runtime();
  failures += test_start_stop_destroy_order();
  failures += test_null_start_is_invalid_argument();
  failures += test_repeated_start_is_bad_state();
  failures += test_repeated_stop_is_bad_state();
  failures += test_init_with_bounded_storage_succeeds();
  failures += test_init_without_bounded_storage_fails();
  failures += test_init_with_bounded_storage_missing_runtime_region_fails();

  return failures == 0 ? 0 : 1;
}
