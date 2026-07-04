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

static_assert(GRCL_C_ABI_VERSION_PATCH == 0u, "unexpected ABI patch version");
static_assert(
  GRCL_C_ABI_VERSION_CURRENT == GRCL_C_ABI_VERSION_PACK(0u, 1u, 0u),
  "ABI version packing drifted");

static_assert(GRCL_RESULT_CATEGORY_GRAPH_SESSION_ERROR > GRCL_RESULT_CATEGORY_SUCCESS,
  "result category ordering drifted");
static_assert(GRCL_ERROR_CHANNEL_REJECTED != GRCL_ERROR_PEER_UNAVAILABLE,
  "graph/session result codes must stay distinct");

static_assert(sizeof(grcl_storage_region_t) >= sizeof(void *) + sizeof(size_t) * 2u,
  "storage region type must remain defined");
static_assert(offsetof(grcl_storage_t, region_count) > offsetof(grcl_storage_t, regions),
  "storage region count layout drifted");
static_assert(GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES != 0u,
  "storage flags drifted");

static_assert(sizeof(grcl_allocator_config_t) >= sizeof(void *) * 3u,
  "allocator config type must remain defined");
static_assert(offsetof(grcl_allocator_config_t, max_allocation_bytes) >
  offsetof(grcl_allocator_config_t, flags),
  "allocator max allocation layout drifted");
static_assert(GRCL_ALLOCATOR_FLAG_NO_HEAP_AFTER_INIT != 0u,
  "allocator flags drifted");

static_assert(sizeof(grcl_diagnostic_record_t) >= sizeof(uint32_t) * 4u,
  "diagnostic record type must remain defined");
static_assert(offsetof(grcl_diagnostic_record_t, scope) > offsetof(grcl_diagnostic_record_t, category),
  "diagnostic scope layout drifted");
static_assert(GRCL_DIAGNOSTIC_CATEGORY_CAPABILITY != GRCL_DIAGNOSTIC_CATEGORY_UNKNOWN,
  "diagnostic categories must stay distinct");

static_assert(sizeof(grcl_runtime_capability_record_t) >= sizeof(size_t) * 10u,
  "capability record type must remain defined");
static_assert(
  offsetof(grcl_runtime_capability_record_t, diagnostics_descriptor_capacity) >
    offsetof(grcl_runtime_capability_record_t, diagnostics_descriptor_count),
  "capability diagnostics descriptor layout drifted");
static_assert(
  GRCL_GRAPH_PROJECTION_MODE_GATEWAY_REPRESENTED >
    GRCL_GRAPH_PROJECTION_MODE_STATIC_MANIFEST,
  "graph projection enum ordering drifted");

static_assert(sizeof(grcl_runtime_capability_request_t) >= sizeof(size_t) * 10u,
  "capability request type must remain defined");
static_assert(
  offsetof(grcl_runtime_capability_request_t, optional_security_descriptor_count) >
    offsetof(grcl_runtime_capability_request_t, required_security_descriptor_count),
  "capability request optional security descriptor layout drifted");
static_assert(
  GRCL_CAPABILITY_SCOPE_GRAPH_PROJECTION > GRCL_CAPABILITY_SCOPE_SESSION,
  "capability scope enum ordering drifted");

static_assert(sizeof(grcl_capability_negotiation_result_t) >= sizeof(size_t) * 10u,
  "capability negotiation result type must remain defined");
static_assert(
  offsetof(grcl_capability_negotiation_result_t, suggested_downgrade_security_descriptor_count) >
    offsetof(grcl_capability_negotiation_result_t, suggested_downgrade_transport_descriptor_count),
  "capability negotiation downgrade descriptor layout drifted");
static_assert(
  GRCL_CAPABILITY_NEGOTIATION_STATUS_DEGRADED_ACCEPTED ==
    static_cast<grcl_capability_negotiation_status_t>(2),
  "capability negotiation status values drifted");

static_assert(sizeof(grcl_runtime_options_t) >= sizeof(void *) * 3u,
  "runtime options type must remain defined");
static_assert(sizeof(grcl_client_options_t) >= sizeof(void *) * 3u,
  "client options type must remain defined");
static_assert(sizeof(grcl_executor_options_t) >= sizeof(void *) * 2u,
  "executor options type must remain defined");
static_assert(offsetof(grcl_client_options_t, response_type_support) >
  offsetof(grcl_client_options_t, request_type_support),
  "client options layout drifted");
static_assert(GRCL_ENDPOINT_OPTION_FLAG_NONE == 0x00000000u,
  "endpoint option flags drifted");

static_assert(sizeof(grcl_backend_runtime_context_t) >= sizeof(void *) * 3u,
  "backend runtime context type must remain defined");
static_assert(
  offsetof(grcl_backend_runtime_context_t, runtime_options) >
    offsetof(grcl_backend_runtime_context_t, abi_version),
  "backend runtime context options layout drifted");
static_assert(
  offsetof(grcl_backend_runtime_context_t, allocator) >
    offsetof(grcl_backend_runtime_context_t, storage),
  "backend runtime context policy fields drifted");

static_assert(sizeof(grcl_backend_descriptor_t) >= sizeof(void *) * 2u,
  "backend descriptor type must remain defined");
static_assert(
  offsetof(grcl_backend_descriptor_t, backend_family_id) >
    offsetof(grcl_backend_descriptor_t, backend_name),
  "backend descriptor family layout drifted");
static_assert(GRCL_BACKEND_FAMILY_ID_NATIVE > GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST,
  "backend family id ordering drifted");
static_assert(GRCL_BACKEND_FAMILY_ID_ROS2 != GRCL_BACKEND_FAMILY_ID_MCU,
  "backend family ids must stay distinct");

static_assert(sizeof(grcl_backend_ops_t) >= sizeof(void *) * 7u,
  "backend ops type must remain defined");
static_assert(offsetof(grcl_backend_ops_t, destroy_runtime) >
  offsetof(grcl_backend_ops_t, stop_runtime),
  "backend lifecycle hook layout drifted");
static_assert(offsetof(grcl_backend_ops_t, negotiate_capabilities) >
  offsetof(grcl_backend_ops_t, get_capabilities),
  "backend capability hook layout drifted");

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

int main()
{
  return (grcl_runtime_get_capabilities_signature != nullptr) &&
    (grcl_runtime_negotiate_capabilities_signature != nullptr) &&
    (grcl_runtime_get_diagnostics_signature != nullptr) &&
    (&grcl_check_backend_ops_signatures != nullptr) ? 0 : 1;
}
