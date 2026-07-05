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

static int expect_flag_unset(const char * label, uint32_t actual, uint32_t flag)
{
  if ((actual & flag) == 0u) {
    return 0;
  }

  (void)label;
  return 1;
}

static grcl_runtime_options_t inprocess_runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "backend-capability-test";
  options.profile_name = "native-inprocess";
  return options;
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
    expect_flag_unset(
      "runtime local params unsupported",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS) != 0 ||
    expect_size("transport descriptor count", record.transport_descriptor_count, 0u) != 0 ||
    expect_size("qos descriptor count", record.qos_descriptor_count, 0u) != 0 ||
    expect_size("encoding descriptor count", record.encoding_descriptor_count, 0u) != 0 ||
    expect_size("security descriptor count", record.security_descriptor_count, 0u) != 0 ||
    expect_size("max parameters", record.max_parameters, 0u) != 0 ||
    expect_size("parameter name buffer bytes", record.parameter_name_buffer_bytes, 0u) != 0 ||
    expect_size("parameter value buffer bytes", record.parameter_value_buffer_bytes, 0u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_query_returns_native_inprocess_capabilities(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_runtime_capability_record_t record = {0};
  grcl_runtime_options_t options = inprocess_runtime_options();
  const uint32_t required_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY |
    GRCL_CAPABILITY_SUMMARY_FLAG_POLL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_SEQUENTIAL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS |
    GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS;

  if (expect_result(
      "create native inprocess runtime",
      grcl_runtime_create(&options, &runtime),
      GRCL_OK) != 0) {
    return 1;
  }

  if (expect_result(
      "query native inprocess capabilities",
      grcl_runtime_get_capabilities(runtime, &record),
      GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_size("native record struct size", record.struct_size, sizeof(record)) != 0 ||
    expect_u32("native record ABI version", record.abi_version, GRCL_C_ABI_VERSION_CURRENT) != 0 ||
    expect_u64("native runtime id", record.runtime_id, 2u) != 0 ||
    expect_u64("native boot id", record.boot_id, 1u) != 0 ||
    expect_u32("native domain id", record.domain_id, 0u) != 0 ||
    expect_u32("native profile id", record.profile_id, 2u) != 0 ||
    expect_u32(
      "native runtime class id",
      record.runtime_class_id,
      GRCL_BACKEND_FAMILY_ID_NATIVE) != 0 ||
    expect_u32(
      "native implementation id",
      record.implementation_id,
      GRCL_BACKEND_FAMILY_ID_NATIVE) != 0 ||
    expect_u32("native implementation version major", record.implementation_version_major, 0u) != 0 ||
    expect_u32("native implementation version minor", record.implementation_version_minor, 1u) != 0 ||
    expect_u32("native implementation version patch", record.implementation_version_patch, 0u) != 0 ||
    expect_u32("native protocol major", record.grcl_protocol_major, 0u) != 0 ||
    expect_u32("native protocol minor", record.grcl_protocol_minor, 1u) != 0 ||
    expect_u32("native capability schema version", record.capability_schema_version, 1u) != 0 ||
    expect_u32(
      "native supported graph projection",
      record.supported_graph_projection_modes,
      GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY) != 0 ||
    expect_u32(
      "native preferred graph projection",
      (uint32_t)record.preferred_graph_projection_mode,
      (uint32_t)GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY) != 0 ||
    expect_u32(
      "native default graph projection",
      (uint32_t)record.default_graph_projection_mode,
      (uint32_t)GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY) != 0 ||
    expect_flag_set("native required summary flags", record.summary_flags, required_flags) != 0 ||
    expect_flag_unset(
      "native static storage unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_STORAGE) != 0 ||
    expect_flag_unset(
      "native no heap after init unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_NO_HEAP_AFTER_INIT) != 0 ||
    expect_flag_unset(
      "native trigger executor unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_TRIGGER_EXECUTOR) != 0 ||
    expect_flag_unset(
      "native multithread executor unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_MULTI_THREAD_EXECUTOR) != 0 ||
    expect_flag_unset(
      "native security family unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_SECURITY_FAMILY_PRESENT) != 0 ||
    expect_flag_unset(
      "native dynamic endpoint delta unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_DYNAMIC_ENDPOINT_DELTA) != 0 ||
    expect_flag_unset(
      "native static manifest unset",
      record.summary_flags,
      GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_MANIFEST) != 0 ||
    expect_size("native max payload bytes", record.max_payload_bytes, 1024u) != 0 ||
    expect_size("native max entities", record.max_entities, 32u) != 0 ||
    expect_size("native max nodes", record.max_nodes, 8u) != 0 ||
    expect_size("native max endpoints", record.max_endpoints, 32u) != 0 ||
    expect_size("native max publishers", record.max_publishers, 16u) != 0 ||
    expect_size("native max subscriptions", record.max_subscriptions, 16u) != 0 ||
    expect_size("native max services non-zero", record.max_services == 0u ? 0u : 1u, 1u) != 0 ||
    expect_size("native max clients non-zero", record.max_clients == 0u ? 0u : 1u, 1u) != 0 ||
    expect_size("native message buffer bytes non-zero", record.message_buffer_bytes == 0u ? 0u : 1u, 1u) != 0 ||
    expect_size("native executor state bytes non-zero", record.executor_state_bytes == 0u ? 0u : 1u, 1u) != 0 ||
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
    expect_size("native parameter name buffer bytes", record.parameter_name_buffer_bytes, 128u) != 0 ||
    expect_size("native parameter value buffer bytes", record.parameter_value_buffer_bytes, 1024u) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy native runtime", grcl_runtime_destroy(runtime), GRCL_OK);
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
  failures += test_query_returns_native_inprocess_capabilities();
  failures += test_start_stop_uses_backend_hooks();
  failures += test_negotiate_compatible_request_is_accepted();
  failures += test_negotiate_optional_unsupported_preference_is_degraded();
  failures += test_negotiate_incompatible_required_request_is_rejected();

  return failures == 0 ? 0 : 1;
}
