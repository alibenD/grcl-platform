#include <cstddef>
#include <cstdio>
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

#include <grcl/c/backend.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
#include <grcl/c/version.h>

#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace {

struct StorageFixture {
  alignas(max_align_t) unsigned char runtime_bytes[512u];
  grcl_storage_region_t runtime_region{};
  grcl_storage_t storage{};
};

struct grcl_runtime {
  enum grcl_runtime_lifecycle_state {
    GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED = 1,
    GRCL_RUNTIME_LIFECYCLE_STATE_STARTED = 2,
    GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED = 3
  } state;

  enum grcl_runtime_storage_ownership {
    GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP = 1,
    GRCL_RUNTIME_STORAGE_OWNERSHIP_CALLER_STORAGE = 2
  } ownership;

  const grcl_runtime_options_t * options;
  const grcl_storage_t * storage;
  const grcl_backend_descriptor_t * backend;
  grcl_backend_runtime_state_t * backend_state;
  int has_latest_diagnostic;
  grcl_diagnostic_record_t latest_diagnostic;
};

static int failures = 0;

static void report_failure(const char * label, const char * detail)
{
  std::fprintf(stderr, "%s: %s\n", label, detail);
  ++failures;
}

static void expect_true(const char * label, bool condition)
{
  if (!condition) {
    report_failure(label, "expected true");
  }
}

static void expect_result(const char * label, grcl::Result actual, grcl::Result expected)
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

static void init_options(grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.flags = GRCL_RUNTIME_OPTION_FLAG_NONE;
  options.runtime_name = "grcl-cpp-smoke";
  options.profile_name = "null/native-test";
  options.allocator = nullptr;
}

static void init_storage(StorageFixture & fixture)
{
  std::memset(fixture.runtime_bytes, 0xA5, sizeof(fixture.runtime_bytes));

  fixture.runtime_region.struct_size = sizeof(fixture.runtime_region);
  fixture.runtime_region.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  fixture.runtime_region.kind = GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT;
  fixture.runtime_region.data = fixture.runtime_bytes;
  fixture.runtime_region.bytes = sizeof(fixture.runtime_bytes);
  fixture.runtime_region.alignment = alignof(max_align_t);
  fixture.runtime_region.flags = GRCL_STORAGE_FLAG_NONE;

  std::memset(&fixture.storage, 0, sizeof(fixture.storage));
  fixture.storage.struct_size = sizeof(fixture.storage);
  fixture.storage.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  fixture.storage.flags =
    GRCL_STORAGE_FLAG_CALLER_PROVIDED_REGIONS |
    GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES;
  fixture.storage.regions = &fixture.runtime_region;
  fixture.storage.region_count = 1u;
  fixture.storage.runtime_object_bytes = sizeof(fixture.runtime_bytes);
}

static const grcl_runtime * as_runtime(const StorageFixture & fixture)
{
  return reinterpret_cast<const grcl_runtime *>(fixture.runtime_bytes);
}

static bool runtime_cleared(const StorageFixture & fixture)
{
  const grcl_runtime * runtime = as_runtime(fixture);
  return runtime->state == 0 &&
    runtime->ownership == 0 &&
    runtime->options == nullptr &&
    runtime->storage == nullptr &&
    runtime->backend == nullptr &&
    runtime->backend_state == nullptr &&
    runtime->has_latest_diagnostic == 0 &&
    runtime->latest_diagnostic.struct_size == 0 &&
    runtime->latest_diagnostic.abi_version == 0 &&
    runtime->latest_diagnostic.code == 0 &&
    runtime->latest_diagnostic.severity == 0 &&
    runtime->latest_diagnostic.category == 0 &&
    runtime->latest_diagnostic.scope == 0 &&
    runtime->latest_diagnostic.detail_flags == 0;
}

static int test_result_alias()
{
  static_assert(
    std::is_same<grcl::Result, grcl_result_t>::value,
    "grcl::Result must alias grcl_result_t");
  return 0;
}

static int test_create_start_stop_destroy()
{
  grcl_runtime_options_t options{};
  grcl::Runtime runtime;

  init_options(options);

  expect_result("runtime create", grcl::Runtime::create(&runtime, &options), GRCL_OK);
  expect_true("runtime owns handle after create", static_cast<bool>(runtime));

  expect_result("runtime start", runtime.start(), GRCL_OK);
  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  expect_true("runtime empty after destroy", !static_cast<bool>(runtime));

  return failures;
}

static int test_move_ownership_and_destruction()
{
  grcl_runtime_options_t options{};
  StorageFixture storage_a{};
  StorageFixture storage_b{};

  init_options(options);
  init_storage(storage_a);
  init_storage(storage_b);

  {
    grcl::Runtime runtime_a;
    grcl::Runtime runtime_b;

    expect_result(
      "runtime_a init_with_storage",
      grcl::Runtime::init_with_storage(&runtime_a, &options, &storage_a.storage),
      GRCL_OK);
    expect_result(
      "runtime_b init_with_storage",
      grcl::Runtime::init_with_storage(&runtime_b, &options, &storage_b.storage),
      GRCL_OK);

    expect_result("runtime_b start before move assignment", runtime_b.start(), GRCL_OK);
    expect_result("runtime_b stop before move assignment", runtime_b.stop(), GRCL_OK);

    runtime_b = std::move(runtime_a);
    expect_true("runtime_a empty after move assignment", !static_cast<bool>(runtime_a));
    expect_true("runtime_b owns moved handle", static_cast<bool>(runtime_b));
    expect_true(
      "runtime_b old storage destroyed on move assignment",
      runtime_cleared(storage_b));

    expect_result("runtime_b start after move assignment", runtime_b.start(), GRCL_OK);
    expect_result("runtime_b stop after move assignment", runtime_b.stop(), GRCL_OK);

    grcl::Runtime runtime_c(std::move(runtime_b));
    expect_true("runtime_b empty after move construction", !static_cast<bool>(runtime_b));
    expect_true("runtime_c owns moved handle", static_cast<bool>(runtime_c));

    expect_result("runtime_c start", runtime_c.start(), GRCL_OK);
    expect_result("runtime_c stop", runtime_c.stop(), GRCL_OK);
    expect_result("runtime_c destroy", runtime_c.destroy(), GRCL_OK);
    expect_true("runtime_c empty after explicit destroy", !static_cast<bool>(runtime_c));
  }

  expect_true(
    "runtime_a storage destroyed on scope exit",
    runtime_cleared(storage_a));

  return failures;
}

}  // namespace

int main()
{
  test_result_alias();
  test_create_start_stop_destroy();
  test_move_ownership_and_destruction();

  if (failures != 0) {
    std::fprintf(stderr, "runtime_wrapper_smoke: %d failure(s)\n", failures);
    return 1;
  }

  std::puts("runtime_wrapper_smoke: ok");
  return 0;
}
