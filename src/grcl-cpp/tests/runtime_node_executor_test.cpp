#include <cstddef>
#include <cstdio>
#include <cstring>
#include <utility>

#include <grcl/c/backend.h>
#include <grcl/c/capability.h>
#include <grcl/c/diagnostics.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
#include <grcl/c/version.h>

#include <grcl/cpp/executor.hpp>
#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace {

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

static void expect_false(const char * label, bool condition)
{
  if (condition) {
    report_failure(label, "expected false");
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

static void expect_ptr(const char * label, const void * actual, const void * expected)
{
  if (actual != expected) {
    report_failure(label, "pointer mismatch");
  }
}

static void expect_u32(const char * label, uint32_t actual, uint32_t expected)
{
  if (actual != expected) {
    report_failure(label, "u32 mismatch");
  }
}

struct StorageFixture {
  alignas(max_align_t) unsigned char runtime_bytes[512u];
  grcl_storage_region_t runtime_region{};
  grcl_storage_t storage{};
};

static void init_options(grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.flags = GRCL_RUNTIME_OPTION_FLAG_NONE;
  options.runtime_name = "grcl-cpp-runtime-node-executor";
  options.profile_name = "null/native-test";
  options.allocator = nullptr;
}

static void init_storage(StorageFixture & fixture)
{
  std::memset(fixture.runtime_bytes, 0xA5, sizeof(fixture.runtime_bytes));
  std::memset(&fixture.runtime_region, 0, sizeof(fixture.runtime_region));
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
  fixture.storage.node_table_capacity = 2u;
  fixture.storage.executor_state_bytes = 128u;
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/cpp";
  return options;
}

static grcl_executor_options_t executor_options()
{
  grcl_executor_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static grcl_runtime_capability_request_t incompatible_request()
{
  grcl_runtime_capability_request_t request{};
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

static int test_runtime_metadata_forwarding()
{
  grcl_runtime_options_t options{};
  grcl::Runtime runtime;
  grcl_runtime_capability_record_t record{};
  grcl_runtime_capability_request_t request = incompatible_request();
  grcl_capability_negotiation_result_t negotiation{};
  grcl_diagnostic_record_t records[1u]{};
  size_t count = 0u;

  init_options(options);

  expect_result("runtime create", grcl::Runtime::create(&runtime, &options), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);
  expect_result("repeated start", runtime.start(), GRCL_ERROR_BAD_STATE);

  std::memset(&record, 0, sizeof(record));
  expect_result("capability query", runtime.get_capabilities(&record), GRCL_OK);
  expect_u32(
    "capability runtime class",
    record.runtime_class_id,
    GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST);

  std::memset(&negotiation, 0, sizeof(negotiation));
  expect_result(
    "capability negotiation",
    runtime.negotiate_capabilities(&request, &negotiation),
    GRCL_OK);
  expect_u32(
    "capability negotiation status",
    static_cast<uint32_t>(negotiation.status),
    static_cast<uint32_t>(GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE));

  expect_result(
    "diagnostics retrieval",
    runtime.get_diagnostics(records, 1u, &count),
    GRCL_OK);
  expect_true("diagnostics count set", count == 1u);
  expect_u32(
    "diagnostic bad state code",
    static_cast<uint32_t>(records[0u].code),
    static_cast<uint32_t>(GRCL_DIAGNOSTIC_CODE_BAD_STATE));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  expect_false("runtime empty after destroy", static_cast<bool>(runtime));

  return failures;
}

static int test_storage_node_executor_flow()
{
  grcl_runtime_options_t options{};
  StorageFixture storage{};
  grcl::Runtime runtime;
  grcl::Node node_a;
  grcl::Node node_b;
  grcl::Executor executor;
  ::grcl_runtime_t * node_runtime = nullptr;
  grcl_node_options_t node_a_options{};
  grcl_node_options_t node_b_options{};
  grcl_executor_options_t executor_create_options{};

  init_options(options);
  init_storage(storage);
  node_a_options = node_options("a");
  node_b_options = node_options("b");
  executor_create_options = executor_options();

  expect_result(
    "runtime init_with_storage",
    grcl::Runtime::init_with_storage(&runtime, &options, &storage.storage),
    GRCL_OK);
  expect_result("node a create", grcl::Node::create(&node_a, &runtime, &node_a_options), GRCL_OK);
  expect_result("node b create", grcl::Node::create(&node_b, &runtime, &node_b_options), GRCL_OK);
  expect_result("node get runtime", node_a.get_runtime(&node_runtime), GRCL_OK);
  expect_ptr("node runtime matches", node_runtime, runtime.get());

  expect_result(
    "executor create",
    grcl::Executor::create(&executor, &runtime, &executor_create_options),
    GRCL_OK);
  expect_result("executor add node a", executor.add_node(&node_a), GRCL_OK);
  expect_result("executor add node b", executor.add_node(&node_b), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);
  expect_result(
    "executor spin once",
    executor.spin_once(0u),
    GRCL_ERROR_UNSUPPORTED_CAPABILITY);
  expect_result("executor remove node a", executor.remove_node(&node_a), GRCL_OK);
  expect_result("executor remove node b", executor.remove_node(&node_b), GRCL_OK);
  expect_result("runtime stop", runtime.stop(), GRCL_OK);

  grcl::Executor moved_executor(std::move(executor));
  expect_false("executor moved from empty", static_cast<bool>(executor));
  expect_true("moved executor owns handle", static_cast<bool>(moved_executor));
  expect_result("destroy moved executor", moved_executor.destroy(), GRCL_OK);

  grcl::Node moved_node(std::move(node_b));
  expect_false("node b moved from empty", static_cast<bool>(node_b));
  expect_true("moved node owns handle", static_cast<bool>(moved_node));
  expect_result("destroy moved node", moved_node.destroy(), GRCL_OK);
  expect_result("destroy node a", node_a.destroy(), GRCL_OK);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);

  return failures;
}

}  // namespace

int main()
{
  test_runtime_metadata_forwarding();
  test_storage_node_executor_flow();

  if (failures != 0) {
    std::fprintf(stderr, "runtime_node_executor_test: %d failure(s)\n", failures);
    return 1;
  }

  std::puts("runtime_node_executor_test: ok");
  return 0;
}
