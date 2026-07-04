#include <stddef.h>

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

static int expect_u64(const char * label, uint64_t actual, uint64_t expected)
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

static int expect_flag_set(const char * label, uint32_t actual, uint32_t flag)
{
  if ((actual & flag) == flag) {
    return 0;
  }

  (void)label;
  return 1;
}

static grcl_runtime_capability_request_t compatible_request(void)
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
  request.required_graph_projection_modes =
    GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY;
  request.preferred_graph_projection_mode =
    GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY;
  request.required_summary_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_STORAGE |
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY;
  return request;
}

static int test_invalid_capability_arguments(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_record_t record = {0};

  if (expect_result(
      "create runtime",
      grcl_runtime_create(NULL, &runtime),
      GRCL_OK) != 0) {
    return 1;
  }

  if (expect_result(
      "null runtime capability query",
      grcl_runtime_get_capabilities(NULL, &record),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "null output capability query",
      grcl_runtime_get_capabilities(runtime, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_query_returns_deterministic_null_native_capabilities(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_record_t record = {0};
  const uint32_t required_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_STORAGE |
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(NULL, &runtime),
      GRCL_OK) != 0) {
    return 1;
  }

  if (expect_result(
      "query capabilities",
      grcl_runtime_get_capabilities(runtime, &record),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_size("record struct size", record.struct_size, sizeof(record)) != 0 ||
    expect_u32("record ABI version", record.abi_version, GRCL_C_ABI_VERSION_CURRENT) != 0 ||
    expect_u64("runtime id", record.runtime_id, 1u) != 0 ||
    expect_u64("boot id", record.boot_id, 1u) != 0 ||
    expect_u32("domain id", record.domain_id, 0u) != 0 ||
    expect_u32("profile id", record.profile_id, 1u) != 0 ||
    expect_u32(
      "runtime class id",
      record.runtime_class_id,
      GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST) != 0 ||
    expect_u32(
      "implementation id",
      record.implementation_id,
      GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST) != 0 ||
    expect_u32("implementation version major", record.implementation_version_major, 0u) != 0 ||
    expect_u32("implementation version minor", record.implementation_version_minor, 1u) != 0 ||
    expect_u32("implementation version patch", record.implementation_version_patch, 0u) != 0 ||
    expect_u32("protocol major", record.grcl_protocol_major, 0u) != 0 ||
    expect_u32("protocol minor", record.grcl_protocol_minor, 1u) != 0 ||
    expect_u32("capability schema version", record.capability_schema_version, 1u) != 0 ||
    expect_u32(
      "supported graph projection",
      record.supported_graph_projection_modes,
      GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY) != 0 ||
    expect_u32(
      "preferred graph projection",
      (uint32_t)record.preferred_graph_projection_mode,
      (uint32_t)GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY) != 0 ||
    expect_u32(
      "default graph projection",
      (uint32_t)record.default_graph_projection_mode,
      (uint32_t)GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY) != 0 ||
    expect_flag_set("required summary flags", record.summary_flags, required_flags) != 0 ||
    expect_size("transport descriptor count", record.transport_descriptor_count, 0u) != 0 ||
    expect_size("qos descriptor count", record.qos_descriptor_count, 0u) != 0 ||
    expect_size("encoding descriptor count", record.encoding_descriptor_count, 0u) != 0 ||
    expect_size("security descriptor count", record.security_descriptor_count, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_start_stop_uses_backend_hooks(void)
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

static int test_negotiate_compatible_request_is_accepted(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_request_t request = compatible_request();
  grcl_capability_negotiation_result_t result = {0};

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "compatible negotiation",
      grcl_runtime_negotiate_capabilities(runtime, &request, &result),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_u32(
      "accepted status",
      (uint32_t)result.status,
      (uint32_t)GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED) != 0 ||
    expect_u64("effective runtime id", result.effective_runtime_id, 1u) != 0 ||
    expect_u32("effective domain id", result.effective_domain_id, 0u) != 0 ||
    expect_u32("effective profile id", result.effective_profile_id, 1u) != 0 ||
    expect_u32(
      "effective runtime class id",
      result.effective_runtime_class_id,
      GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST) != 0 ||
    expect_u32("suggested downgrade absent", result.suggested_downgrade_present, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_negotiate_optional_unsupported_preference_is_degraded(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_request_t request = compatible_request();
  grcl_capability_negotiation_result_t result = {0};
  request.optional_graph_projection_modes = GRCL_GRAPH_PROJECTION_MODE_FLAG_FULL;
  request.preferred_graph_projection_mode = GRCL_GRAPH_PROJECTION_MODE_FULL;
  request.optional_summary_flags = GRCL_CAPABILITY_SUMMARY_FLAG_SECURITY_FAMILY_PRESENT;
  request.optional_transport_descriptor_count = 1u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "degraded negotiation",
      grcl_runtime_negotiate_capabilities(runtime, &request, &result),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_u32(
      "degraded status",
      (uint32_t)result.status,
      (uint32_t)GRCL_CAPABILITY_NEGOTIATION_STATUS_DEGRADED_ACCEPTED) != 0 ||
    expect_u32("suggested downgrade present", result.suggested_downgrade_present, 1u) != 0 ||
    expect_u32(
      "downgrade graph projection",
      (uint32_t)result.suggested_downgrade_graph_projection_mode,
      (uint32_t)GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY) != 0 ||
    expect_size(
      "downgrade transport descriptors",
      result.suggested_downgrade_transport_descriptor_count,
      0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_negotiate_incompatible_required_request_is_rejected(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_request_t request = compatible_request();
  grcl_capability_negotiation_result_t result = {0};
  request.required_graph_projection_modes = GRCL_GRAPH_PROJECTION_MODE_FLAG_FULL;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "incompatible negotiation",
      grcl_runtime_negotiate_capabilities(runtime, &request, &result),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_u32(
      "rejected status",
      (uint32_t)result.status,
      (uint32_t)GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE) != 0 ||
    expect_u32("suggested downgrade absent", result.suggested_downgrade_present, 0u) != 0 ||
    expect_size("rejection reason count", result.rejection_reason_count, 1u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  int failures = 0;

  failures += test_invalid_capability_arguments();
  failures += test_query_returns_deterministic_null_native_capabilities();
  failures += test_start_stop_uses_backend_hooks();
  failures += test_negotiate_compatible_request_is_accepted();
  failures += test_negotiate_optional_unsupported_preference_is_degraded();
  failures += test_negotiate_incompatible_required_request_is_rejected();

  return failures == 0 ? 0 : 1;
}
