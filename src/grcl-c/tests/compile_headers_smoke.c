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

static void grcl_check_backend_ops_signatures(const grcl_backend_ops_t * ops)
{
  grcl_result_t (*get_diagnostics)(
    const grcl_backend_runtime_state_t *,
    grcl_diagnostic_record_t *,
    size_t,
    size_t *) = ops->get_diagnostics;
  (void)get_diagnostics;
}

int main(void)
{
  return (grcl_runtime_get_capabilities_signature != 0) &&
    (grcl_runtime_negotiate_capabilities_signature != 0) &&
    (grcl_runtime_get_diagnostics_signature != 0) &&
    (&grcl_check_backend_ops_signatures != 0) ? 0 : 1;
}
