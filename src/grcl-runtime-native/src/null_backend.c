#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/version.h>

struct grcl_backend_runtime_state {
  int started;
  const grcl_storage_t * storage;
};

static void grcl_null_native_fill_capabilities(
  grcl_runtime_capability_record_t * record)
{
  memset(record, 0, sizeof(*record));
  record->struct_size = sizeof(*record);
  record->abi_version = GRCL_C_ABI_VERSION_CURRENT;
  record->runtime_id = 1u;
  record->boot_id = 1u;
  record->domain_id = 0u;
  record->profile_id = 1u;
  record->runtime_class_id = GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST;
  record->implementation_id = GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST;
  record->implementation_version_major = 0u;
  record->implementation_version_minor = 1u;
  record->implementation_version_patch = 0u;
  record->grcl_protocol_major = 0u;
  record->grcl_protocol_minor = 1u;
  record->capability_schema_version = 1u;
  record->supported_graph_projection_modes =
    GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY;
  record->preferred_graph_projection_mode =
    GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY;
  record->default_graph_projection_mode =
    GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY;
  record->summary_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_STORAGE |
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY |
    GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS;
  record->max_payload_bytes = 0u;
  record->max_entities = 1u;
  record->max_nodes = 0u;
  record->max_endpoints = 0u;
  record->max_publishers = 0u;
  record->max_subscriptions = 0u;
  record->max_services = 0u;
  record->max_clients = 0u;
  record->max_sessions = 0u;
  record->max_channels = 0u;
  record->message_buffer_bytes = 0u;
  record->graph_cache_bytes = 0u;
  record->diagnostics_buffer_bytes = 0u;
  record->history_buffer_bytes = 0u;
  record->executor_state_bytes = 0u;
  record->storage_region_count = 0u;
  record->storage_region_capacity = 1u;
}

static int grcl_null_native_protocol_below_min(
  const grcl_runtime_capability_record_t * record,
  const grcl_runtime_capability_request_t * request)
{
  return request->min_grcl_protocol_major > record->grcl_protocol_major ||
    (request->min_grcl_protocol_major == record->grcl_protocol_major &&
    request->min_grcl_protocol_minor > record->grcl_protocol_minor);
}

static int grcl_null_native_protocol_above_max(
  const grcl_runtime_capability_record_t * record,
  const grcl_runtime_capability_request_t * request)
{
  if (request->max_grcl_protocol_major == 0u &&
    request->max_grcl_protocol_minor == 0u) {
    return 0;
  }

  return request->max_grcl_protocol_major < record->grcl_protocol_major ||
    (request->max_grcl_protocol_major == record->grcl_protocol_major &&
    request->max_grcl_protocol_minor < record->grcl_protocol_minor);
}

static int grcl_null_native_flags_missing(uint32_t supported, uint32_t requested)
{
  return requested != 0u && (supported & requested) != requested;
}

static int grcl_null_native_required_incompatible(
  const grcl_runtime_capability_record_t * record,
  const grcl_runtime_capability_request_t * request)
{
  if (request->target_runtime_id != 0u &&
    request->target_runtime_id != record->runtime_id) {
    return 1;
  }
  if (request->target_domain_id != record->domain_id) {
    return 1;
  }
  if (request->required_profile_id != 0u &&
    request->required_profile_id != record->profile_id) {
    return 1;
  }
  if (request->required_runtime_class_id != 0u &&
    request->required_runtime_class_id != record->runtime_class_id) {
    return 1;
  }
  if (grcl_null_native_protocol_below_min(record, request) ||
    grcl_null_native_protocol_above_max(record, request) ||
    request->min_capability_schema_version > record->capability_schema_version) {
    return 1;
  }
  if (grcl_null_native_flags_missing(
      record->supported_graph_projection_modes,
      request->required_graph_projection_modes) ||
    grcl_null_native_flags_missing(
      record->summary_flags,
      request->required_summary_flags)) {
    return 1;
  }
  if (request->required_min_payload_bytes > record->max_payload_bytes ||
    request->required_min_entities > record->max_entities ||
    request->required_min_nodes > record->max_nodes ||
    request->required_min_endpoints > record->max_endpoints ||
    request->required_min_sessions > record->max_sessions ||
    request->required_min_channels > record->max_channels ||
    request->required_min_message_buffer_bytes > record->message_buffer_bytes ||
    request->required_transport_descriptor_count > record->transport_descriptor_count ||
    request->required_qos_descriptor_count > record->qos_descriptor_count ||
    request->required_encoding_descriptor_count > record->encoding_descriptor_count ||
    request->required_diagnostics_descriptor_count > record->diagnostics_descriptor_count ||
    request->required_security_descriptor_count > record->security_descriptor_count) {
    return 1;
  }

  return 0;
}

static int grcl_null_native_optional_degraded(
  const grcl_runtime_capability_record_t * record,
  const grcl_runtime_capability_request_t * request)
{
  if (request->optional_profile_id != 0u &&
    request->optional_profile_id != record->profile_id) {
    return 1;
  }
  if (request->optional_runtime_class_id != 0u &&
    request->optional_runtime_class_id != record->runtime_class_id) {
    return 1;
  }
  if (grcl_null_native_flags_missing(
      record->supported_graph_projection_modes,
      request->optional_graph_projection_modes) ||
    grcl_null_native_flags_missing(
      record->summary_flags,
      request->optional_summary_flags)) {
    return 1;
  }
  if (request->preferred_graph_projection_mode != GRCL_GRAPH_PROJECTION_MODE_UNKNOWN &&
    request->preferred_graph_projection_mode != record->preferred_graph_projection_mode) {
    return 1;
  }
  if (request->optional_min_payload_bytes > record->max_payload_bytes ||
    request->optional_min_entities > record->max_entities ||
    request->optional_min_nodes > record->max_nodes ||
    request->optional_min_endpoints > record->max_endpoints ||
    request->optional_min_sessions > record->max_sessions ||
    request->optional_min_channels > record->max_channels ||
    request->optional_min_message_buffer_bytes > record->message_buffer_bytes ||
    request->optional_transport_descriptor_count > record->transport_descriptor_count ||
    request->optional_qos_descriptor_count > record->qos_descriptor_count ||
    request->optional_encoding_descriptor_count > record->encoding_descriptor_count ||
    request->optional_diagnostics_descriptor_count > record->diagnostics_descriptor_count ||
    request->optional_security_descriptor_count > record->security_descriptor_count) {
    return 1;
  }

  return 0;
}

static void grcl_null_native_fill_negotiation_effective(
  const grcl_runtime_capability_record_t * record,
  grcl_capability_negotiation_result_t * result)
{
  result->effective_runtime_id = record->runtime_id;
  result->effective_domain_id = record->domain_id;
  result->effective_profile_id = record->profile_id;
  result->effective_runtime_class_id = record->runtime_class_id;
  result->effective_grcl_protocol_major = record->grcl_protocol_major;
  result->effective_grcl_protocol_minor = record->grcl_protocol_minor;
  result->effective_capability_schema_version = record->capability_schema_version;
  result->effective_graph_projection_mode = record->default_graph_projection_mode;
  result->effective_summary_flags = record->summary_flags;
  result->effective_max_payload_bytes = record->max_payload_bytes;
  result->effective_max_entities = record->max_entities;
  result->effective_max_nodes = record->max_nodes;
  result->effective_max_endpoints = record->max_endpoints;
  result->effective_max_sessions = record->max_sessions;
  result->effective_max_channels = record->max_channels;
  result->effective_transport_descriptor_count =
    record->transport_descriptor_count;
  result->effective_qos_descriptor_count = record->qos_descriptor_count;
  result->effective_encoding_descriptor_count = record->encoding_descriptor_count;
  result->effective_diagnostics_descriptor_count =
    record->diagnostics_descriptor_count;
  result->effective_security_descriptor_count = record->security_descriptor_count;
}

static void grcl_null_native_fill_suggested_downgrade(
  const grcl_runtime_capability_record_t * record,
  grcl_capability_negotiation_result_t * result)
{
  result->suggested_downgrade_present = 1u;
  result->suggested_downgrade_scope = GRCL_CAPABILITY_SCOPE_SESSION;
  result->suggested_downgrade_graph_projection_mode =
    record->default_graph_projection_mode;
  result->suggested_downgrade_summary_flags = record->summary_flags;
  result->suggested_downgrade_max_payload_bytes = record->max_payload_bytes;
  result->suggested_downgrade_transport_descriptor_count =
    record->transport_descriptor_count;
  result->suggested_downgrade_qos_descriptor_count =
    record->qos_descriptor_count;
  result->suggested_downgrade_encoding_descriptor_count =
    record->encoding_descriptor_count;
  result->suggested_downgrade_diagnostics_descriptor_count =
    record->diagnostics_descriptor_count;
  result->suggested_downgrade_security_descriptor_count =
    record->security_descriptor_count;
}

static grcl_result_t grcl_null_native_create_runtime(
  const grcl_backend_runtime_context_t * context,
  grcl_backend_runtime_state_t ** out_backend_state)
{
  grcl_backend_runtime_state_t * state = NULL;

  if (context == NULL || out_backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_state = NULL;
  state = (grcl_backend_runtime_state_t *)calloc(1u, sizeof(*state));
  if (state == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  state->storage = context->storage;
  *out_backend_state = state;
  return GRCL_OK;
}

static grcl_result_t grcl_null_native_start_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  backend_state->started = 1;
  return GRCL_OK;
}

static grcl_result_t grcl_null_native_stop_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  backend_state->started = 0;
  return GRCL_OK;
}

static grcl_result_t grcl_null_native_destroy_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  free(backend_state);
  return GRCL_OK;
}

static grcl_result_t grcl_null_native_get_capabilities(
  const grcl_backend_runtime_state_t * backend_state,
  grcl_runtime_capability_record_t * out_record)
{
  if (backend_state == NULL || out_record == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_null_native_fill_capabilities(out_record);
  return GRCL_OK;
}

static grcl_result_t grcl_null_native_negotiate_capabilities(
  const grcl_backend_runtime_state_t * backend_state,
  const grcl_runtime_capability_request_t * request,
  grcl_capability_negotiation_result_t * out_result)
{
  grcl_runtime_capability_record_t record;

  if (backend_state == NULL || request == NULL || out_result == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_null_native_fill_capabilities(&record);
  memset(out_result, 0, sizeof(*out_result));
  out_result->struct_size = sizeof(*out_result);
  out_result->abi_version = GRCL_C_ABI_VERSION_CURRENT;
  out_result->scope = request->scope == GRCL_CAPABILITY_SCOPE_UNKNOWN ?
    GRCL_CAPABILITY_SCOPE_SESSION : request->scope;
  grcl_null_native_fill_negotiation_effective(&record, out_result);

  if (grcl_null_native_required_incompatible(&record, request)) {
    out_result->status =
      GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE;
    out_result->result_category = GRCL_RESULT_CATEGORY_COMPATIBILITY_ERROR;
    out_result->rejection_reason_count = 1u;
    out_result->rejection_reason_capacity = 1u;
    return GRCL_OK;
  }

  out_result->result_category = GRCL_RESULT_CATEGORY_SUCCESS;
  if (grcl_null_native_optional_degraded(&record, request)) {
    out_result->status =
      GRCL_CAPABILITY_NEGOTIATION_STATUS_DEGRADED_ACCEPTED;
    grcl_null_native_fill_suggested_downgrade(&record, out_result);
  } else {
    out_result->status = GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED;
  }

  return GRCL_OK;
}

static grcl_result_t grcl_null_native_get_diagnostics(
  const grcl_backend_runtime_state_t * backend_state,
  grcl_diagnostic_record_t * out_records,
  size_t record_capacity,
  size_t * out_record_count)
{
  if (backend_state == NULL || out_record_count == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (record_capacity > 0u && out_records == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_record_count = 0u;
  return GRCL_OK;
}

static const grcl_backend_ops_t grcl_null_native_backend_ops = {
  sizeof(grcl_backend_ops_t),
  GRCL_C_ABI_VERSION_CURRENT,
  grcl_null_native_create_runtime,
  grcl_null_native_start_runtime,
  grcl_null_native_stop_runtime,
  grcl_null_native_destroy_runtime,
  grcl_null_native_get_capabilities,
  grcl_null_native_negotiate_capabilities,
  grcl_null_native_get_diagnostics
};

static const grcl_backend_descriptor_t grcl_null_native_backend_descriptor = {
  sizeof(grcl_backend_descriptor_t),
  GRCL_C_ABI_VERSION_CURRENT,
  "null/native-test",
  GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST,
  0u,
  1u,
  0u,
  &grcl_null_native_backend_ops
};

const grcl_backend_descriptor_t * grcl_null_native_test_backend_descriptor(void)
{
  return &grcl_null_native_backend_descriptor;
}
