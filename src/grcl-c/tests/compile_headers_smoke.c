#include <stddef.h>

#include <grcl/c/allocator.h>
#include <grcl/c/backend.h>
#include <grcl/c/capability.h>
#include <grcl/c/diagnostics.h>
#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
#include <grcl/c/types.h>
#include <grcl/c/version.h>

#define GRCL_STATIC_ASSERT(condition, message) _Static_assert((condition), message)

GRCL_STATIC_ASSERT(GRCL_C_ABI_VERSION_MAJOR == 0u, "unexpected ABI major version");
GRCL_STATIC_ASSERT(GRCL_C_ABI_VERSION_MINOR == 1u, "unexpected ABI minor version");
GRCL_STATIC_ASSERT(
  GRCL_C_ABI_VERSION_CURRENT == GRCL_C_ABI_VERSION_PACK(0u, 1u, 0u),
  "ABI version packing drifted");

GRCL_STATIC_ASSERT(GRCL_RESULT_CATEGORY_SUCCESS != GRCL_RESULT_CATEGORY_CALLER_ERROR,
  "result categories must stay distinct");
GRCL_STATIC_ASSERT(GRCL_ERROR_INVALID_ARGUMENT != GRCL_OK,
  "result codes must stay distinct");
GRCL_STATIC_ASSERT(GRCL_ERROR_BACKEND_FAILURE > GRCL_ERROR_TIMEOUT,
  "runtime error ordering drifted");
GRCL_STATIC_ASSERT(GRCL_ERROR_NOT_FOUND == 0x0103,
  "M3 not-found result code drifted");
GRCL_STATIC_ASSERT(GRCL_ERROR_NO_DATA == 0x0404,
  "M3 no-data result code drifted");
GRCL_STATIC_ASSERT(GRCL_ERROR_TYPE_MISMATCH == 0x0303,
  "M3 type-mismatch result code drifted");
GRCL_STATIC_ASSERT(GRCL_ERROR_PAYLOAD_TOO_LARGE == 0x0203,
  "M3 payload-too-large result code drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_storage_region_t) >= sizeof(size_t) * 3u,
  "storage region type must remain defined");
GRCL_STATIC_ASSERT(offsetof(grcl_storage_region_t, abi_version) > offsetof(grcl_storage_region_t, struct_size),
  "storage region ABI fields drifted");
GRCL_STATIC_ASSERT(offsetof(grcl_storage_t, diagnostics_buffer_bytes) > offsetof(grcl_storage_t, graph_cache_bytes),
  "storage buffer layout drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_allocator_config_t) >= sizeof(void *) * 3u,
  "allocator config type must remain defined");
GRCL_STATIC_ASSERT(offsetof(grcl_allocator_config_t, allocate) > offsetof(grcl_allocator_config_t, user_data),
  "allocator callback layout drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_diagnostic_record_t) >= sizeof(uint32_t) * 4u,
  "diagnostic record type must remain defined");
GRCL_STATIC_ASSERT(offsetof(grcl_diagnostic_record_t, detail_flags) > offsetof(grcl_diagnostic_record_t, code),
  "diagnostic record layout drifted");
GRCL_STATIC_ASSERT(GRCL_DIAGNOSTIC_CODE_STORAGE_REGION_MISSING != GRCL_DIAGNOSTIC_CODE_NONE,
  "diagnostic codes must stay distinct");

GRCL_STATIC_ASSERT(sizeof(grcl_runtime_capability_record_t) >= sizeof(uint64_t) * 2u,
  "capability record type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_record_t, preferred_graph_projection_mode) >
    offsetof(grcl_runtime_capability_record_t, supported_graph_projection_modes),
  "capability record graph fields drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_record_t, transport_descriptor_capacity) >
    offsetof(grcl_runtime_capability_record_t, transport_descriptor_count),
  "capability record descriptor fields drifted");
GRCL_STATIC_ASSERT(
  (GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS & GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_MANIFEST) == 0u,
  "runtime-local params capability flag must remain distinct");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_record_t, max_parameters) >
    offsetof(grcl_runtime_capability_record_t, max_clients),
  "runtime-local params capability fields drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_record_t, parameter_value_buffer_bytes) >
    offsetof(grcl_runtime_capability_record_t, parameter_name_buffer_bytes),
  "parameter capability buffer layout drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_runtime_capability_request_t) >= sizeof(size_t) * 8u,
  "capability request type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_request_t, preferred_graph_projection_mode) >
    offsetof(grcl_runtime_capability_request_t, optional_graph_projection_modes),
  "capability request graph preference layout drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_runtime_capability_request_t, required_security_descriptor_count) >
    offsetof(grcl_runtime_capability_request_t, required_transport_descriptor_count),
  "capability request descriptor counts drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_capability_negotiation_result_t) >= sizeof(size_t) * 8u,
  "capability negotiation result type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_capability_negotiation_result_t, effective_summary_flags) >
    offsetof(grcl_capability_negotiation_result_t, effective_graph_projection_mode),
  "capability negotiation result layout drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_capability_negotiation_result_t, suggested_downgrade_present) >
    offsetof(grcl_capability_negotiation_result_t, rejection_reason_capacity),
  "capability negotiation downgrade fields drifted");
GRCL_STATIC_ASSERT(
  GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED == 1 &&
    GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE == 3,
  "capability negotiation status values drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_runtime_options_t) >= sizeof(void *) * 3u,
  "runtime options type must remain defined");
GRCL_STATIC_ASSERT(sizeof(grcl_publisher_options_t) >= sizeof(void *) * 2u,
  "publisher options type must remain defined");
GRCL_STATIC_ASSERT(sizeof(grcl_subscription_options_t) >= sizeof(void *) * 2u,
  "subscription options type must remain defined");
GRCL_STATIC_ASSERT(sizeof(grcl_type_support_t) >= sizeof(size_t) + sizeof(uint32_t) * 2u,
  "type support descriptor must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_type_support_t, type_name) > offsetof(grcl_type_support_t, type_id),
  "type support identity fields drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_type_support_t, alignment) > offsetof(grcl_type_support_t, fixed_size),
  "type support size fields drifted");
GRCL_STATIC_ASSERT(GRCL_TYPE_SUPPORT_FLAG_FIXED_SIZE == 0x00000001u,
  "type support flags drifted");
GRCL_STATIC_ASSERT(sizeof(grcl_request_id_t) == sizeof(uint64_t),
  "request id type drifted");
GRCL_STATIC_ASSERT(GRCL_PARAM_TYPE_BYTES == 0x0006,
  "parameter type enum drifted");
GRCL_STATIC_ASSERT(sizeof(grcl_param_record_t) >= sizeof(void *) * 2u,
  "parameter record type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_param_record_t, value_size) > offsetof(grcl_param_record_t, value),
  "parameter record value fields drifted");
GRCL_STATIC_ASSERT(GRCL_ENDPOINT_OPTION_FLAG_NONE == 0x00000000u,
  "endpoint option flags drifted");
GRCL_STATIC_ASSERT(offsetof(grcl_runtime_options_t, allocator) > offsetof(grcl_runtime_options_t, profile_name),
  "runtime options layout drifted");
GRCL_STATIC_ASSERT(offsetof(grcl_publisher_options_t, type_support) > offsetof(grcl_publisher_options_t, topic_name),
  "publisher options layout drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_backend_runtime_context_t) >= sizeof(void *) * 3u,
  "backend runtime context type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_runtime_context_t, abi_version) >
    offsetof(grcl_backend_runtime_context_t, struct_size),
  "backend runtime context ABI fields drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_runtime_context_t, allocator) >
    offsetof(grcl_backend_runtime_context_t, storage),
  "backend runtime context policy fields drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_backend_descriptor_t) >= sizeof(void *) * 2u,
  "backend descriptor type must remain defined");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_descriptor_t, ops) > offsetof(grcl_backend_descriptor_t, backend_version_patch),
  "backend descriptor ops field drifted");
GRCL_STATIC_ASSERT(GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST != GRCL_BACKEND_FAMILY_ID_UNKNOWN,
  "backend family ids must stay distinct");
GRCL_STATIC_ASSERT(GRCL_BACKEND_FAMILY_ID_SIMULATOR > GRCL_BACKEND_FAMILY_ID_GATEWAY,
  "backend family id ordering drifted");

GRCL_STATIC_ASSERT(sizeof(grcl_backend_ops_t) >= sizeof(void *) * 7u,
  "backend ops type must remain defined");
GRCL_STATIC_ASSERT(offsetof(grcl_backend_ops_t, create_runtime) > offsetof(grcl_backend_ops_t, abi_version),
  "backend ops ABI fields drifted");
GRCL_STATIC_ASSERT(offsetof(grcl_backend_ops_t, get_diagnostics) > offsetof(grcl_backend_ops_t, get_capabilities),
  "backend diagnostics hook layout drifted");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_ops_t, create_node) > offsetof(grcl_backend_ops_t, get_diagnostics),
  "M3 backend node hook must append after M1 hooks");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_ops_t, runtime_param_set) > offsetof(grcl_backend_ops_t, executor_spin_once),
  "M3 backend param hooks must append after executor hooks");
GRCL_STATIC_ASSERT(
  offsetof(grcl_backend_ops_t, runtime_param_list) > offsetof(grcl_backend_ops_t, runtime_param_get),
  "M3 backend param list hook layout drifted");

static grcl_result_t (*const grcl_runtime_get_capabilities_signature)(
  const grcl_runtime_t *,
  grcl_runtime_capability_record_t *) = &grcl_runtime_get_capabilities;

static grcl_result_t (*const grcl_runtime_negotiate_capabilities_signature)(
  const grcl_runtime_t *,
  const grcl_runtime_capability_request_t *,
  grcl_capability_negotiation_result_t *) = &grcl_runtime_negotiate_capabilities;

static grcl_result_t (*const grcl_runtime_get_diagnostics_signature)(
  const grcl_runtime_t *,
  grcl_diagnostic_record_t *,
  size_t,
  size_t *) = &grcl_runtime_get_diagnostics;

static grcl_result_t (*const grcl_publisher_publish_bytes_signature)(
  grcl_publisher_t *,
  const void *,
  size_t) = &grcl_publisher_publish_bytes;

static grcl_result_t (*const grcl_subscription_take_bytes_signature)(
  grcl_subscription_t *,
  void *,
  size_t,
  size_t *) = &grcl_subscription_take_bytes;

static grcl_result_t (*const grcl_client_send_request_bytes_signature)(
  grcl_client_t *,
  const void *,
  size_t,
  grcl_request_id_t *) = &grcl_client_send_request_bytes;

static grcl_result_t (*const grcl_service_take_request_bytes_signature)(
  grcl_service_t *,
  void *,
  size_t,
  size_t *,
  grcl_request_id_t *) = &grcl_service_take_request_bytes;

static grcl_result_t (*const grcl_service_send_response_bytes_signature)(
  grcl_service_t *,
  grcl_request_id_t,
  const void *,
  size_t) = &grcl_service_send_response_bytes;

static grcl_result_t (*const grcl_client_take_response_bytes_signature)(
  grcl_client_t *,
  grcl_request_id_t,
  void *,
  size_t,
  size_t *) = &grcl_client_take_response_bytes;

static grcl_result_t (*const grcl_runtime_param_set_signature)(
  grcl_runtime_t *,
  const grcl_param_record_t *) = &grcl_runtime_param_set;

static grcl_result_t (*const grcl_runtime_param_get_signature)(
  grcl_runtime_t *,
  const char *,
  grcl_param_record_t *,
  void *,
  size_t,
  size_t *) = &grcl_runtime_param_get;

static grcl_result_t (*const grcl_runtime_param_list_signature)(
  grcl_runtime_t *,
  char *,
  size_t,
  size_t *,
  size_t *) = &grcl_runtime_param_list;

static void grcl_check_backend_ops_signatures(const grcl_backend_ops_t * ops)
{
  grcl_result_t (*get_diagnostics)(
    const grcl_backend_runtime_state_t *,
    grcl_diagnostic_record_t *,
    size_t,
    size_t *) = ops->get_diagnostics;
  grcl_result_t (*create_node)(
    grcl_backend_runtime_state_t *,
    const grcl_node_t *,
    const grcl_node_options_t *,
    grcl_backend_node_state_t **) = ops->create_node;
  grcl_result_t (*publish_bytes)(
    grcl_backend_runtime_state_t *,
    grcl_backend_publisher_state_t *,
    const void *,
    size_t) = ops->publish_bytes;
  grcl_result_t (*client_send_request_bytes)(
    grcl_backend_runtime_state_t *,
    grcl_backend_client_state_t *,
    const void *,
    size_t,
    grcl_request_id_t *) = ops->client_send_request_bytes;
  grcl_result_t (*executor_spin_once)(
    grcl_backend_runtime_state_t *,
    grcl_backend_executor_state_t *,
    uint64_t) = ops->executor_spin_once;
  grcl_result_t (*runtime_param_list)(
    grcl_backend_runtime_state_t *,
    char *,
    size_t,
    size_t *,
    size_t *) = ops->runtime_param_list;
  (void)get_diagnostics;
  (void)create_node;
  (void)publish_bytes;
  (void)client_send_request_bytes;
  (void)executor_spin_once;
  (void)runtime_param_list;
}

int main(void)
{
  return (grcl_runtime_get_capabilities_signature != 0) &&
    (grcl_runtime_negotiate_capabilities_signature != 0) &&
    (grcl_runtime_get_diagnostics_signature != 0) &&
    (grcl_publisher_publish_bytes_signature != 0) &&
    (grcl_subscription_take_bytes_signature != 0) &&
    (grcl_client_send_request_bytes_signature != 0) &&
    (grcl_service_take_request_bytes_signature != 0) &&
    (grcl_service_send_response_bytes_signature != 0) &&
    (grcl_client_take_response_bytes_signature != 0) &&
    (grcl_runtime_param_set_signature != 0) &&
    (grcl_runtime_param_get_signature != 0) &&
    (grcl_runtime_param_list_signature != 0) &&
    (&grcl_check_backend_ops_signatures != 0) ? 0 : 1;
}
