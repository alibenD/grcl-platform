#ifndef GRCL_C_CAPABILITY_H_
#define GRCL_C_CAPABILITY_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum grcl_capability_negotiation_status {
  GRCL_CAPABILITY_NEGOTIATION_STATUS_UNKNOWN = 0,
  GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED = 1,
  GRCL_CAPABILITY_NEGOTIATION_STATUS_DEGRADED_ACCEPTED = 2,
  GRCL_CAPABILITY_NEGOTIATION_STATUS_REJECTED_INCOMPATIBLE = 3
} grcl_capability_negotiation_status_t;

typedef enum grcl_graph_projection_mode {
  GRCL_GRAPH_PROJECTION_MODE_UNKNOWN = 0,
  GRCL_GRAPH_PROJECTION_MODE_FULL = 1,
  GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY = 2,
  GRCL_GRAPH_PROJECTION_MODE_INTEREST_SCOPED = 3,
  GRCL_GRAPH_PROJECTION_MODE_STATIC_MANIFEST = 4,
  GRCL_GRAPH_PROJECTION_MODE_GATEWAY_REPRESENTED = 5
} grcl_graph_projection_mode_t;

typedef enum grcl_capability_scope {
  GRCL_CAPABILITY_SCOPE_UNKNOWN = 0,
  GRCL_CAPABILITY_SCOPE_SESSION = 1,
  GRCL_CAPABILITY_SCOPE_CHANNEL = 2,
  GRCL_CAPABILITY_SCOPE_ENDPOINT = 3,
  GRCL_CAPABILITY_SCOPE_GRAPH_PROJECTION = 4
} grcl_capability_scope_t;

typedef enum grcl_graph_projection_mode_flags {
  GRCL_GRAPH_PROJECTION_MODE_FLAG_NONE = 0x00000000u,
  GRCL_GRAPH_PROJECTION_MODE_FLAG_FULL = 0x00000001u,
  GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY = 0x00000002u,
  GRCL_GRAPH_PROJECTION_MODE_FLAG_INTEREST_SCOPED = 0x00000004u,
  GRCL_GRAPH_PROJECTION_MODE_FLAG_STATIC_MANIFEST = 0x00000008u,
  GRCL_GRAPH_PROJECTION_MODE_FLAG_GATEWAY_REPRESENTED = 0x00000010u
} grcl_graph_projection_mode_flags_t;

typedef enum grcl_capability_summary_flags {
  GRCL_CAPABILITY_SUMMARY_FLAG_NONE = 0x00000000u,
  GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_STORAGE = 0x00000001u,
  GRCL_CAPABILITY_SUMMARY_FLAG_NO_HEAP_AFTER_INIT = 0x00000002u,
  GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES = 0x00000004u,
  GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY = 0x00000008u,
  GRCL_CAPABILITY_SUMMARY_FLAG_POLL_EXECUTOR = 0x00000010u,
  GRCL_CAPABILITY_SUMMARY_FLAG_SEQUENTIAL_EXECUTOR = 0x00000020u,
  GRCL_CAPABILITY_SUMMARY_FLAG_TRIGGER_EXECUTOR = 0x00000040u,
  GRCL_CAPABILITY_SUMMARY_FLAG_MULTI_THREAD_EXECUTOR = 0x00000080u,
  GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS = 0x00000100u,
  GRCL_CAPABILITY_SUMMARY_FLAG_EXTENDED_DIAGNOSTICS = 0x00000200u,
  GRCL_CAPABILITY_SUMMARY_FLAG_SECURITY_FAMILY_PRESENT = 0x00000400u,
  GRCL_CAPABILITY_SUMMARY_FLAG_DYNAMIC_ENDPOINT_DELTA = 0x00000800u,
  GRCL_CAPABILITY_SUMMARY_FLAG_STATIC_MANIFEST = 0x00001000u,
  GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS = 0x00002000u
} grcl_capability_summary_flags_t;

typedef struct grcl_runtime_capability_record {
  size_t struct_size;
  uint32_t abi_version;
  uint64_t runtime_id;
  uint64_t boot_id;
  uint32_t domain_id;
  uint32_t profile_id;
  uint32_t runtime_class_id;
  uint32_t implementation_id;
  uint32_t implementation_version_major;
  uint32_t implementation_version_minor;
  uint32_t implementation_version_patch;
  uint32_t grcl_protocol_major;
  uint32_t grcl_protocol_minor;
  uint32_t capability_schema_version;
  uint32_t supported_graph_projection_modes;
  grcl_graph_projection_mode_t preferred_graph_projection_mode;
  grcl_graph_projection_mode_t default_graph_projection_mode;
  uint32_t summary_flags;
  size_t max_payload_bytes;
  size_t max_entities;
  size_t max_nodes;
  size_t max_endpoints;
  size_t max_publishers;
  size_t max_subscriptions;
  size_t max_services;
  size_t max_clients;
  size_t max_sessions;
  size_t max_channels;
  size_t message_buffer_bytes;
  size_t graph_cache_bytes;
  size_t diagnostics_buffer_bytes;
  size_t history_buffer_bytes;
  size_t executor_state_bytes;
  size_t storage_region_count;
  size_t storage_region_capacity;
  size_t transport_descriptor_count;
  size_t transport_descriptor_capacity;
  size_t qos_descriptor_count;
  size_t qos_descriptor_capacity;
  size_t encoding_descriptor_count;
  size_t encoding_descriptor_capacity;
  size_t diagnostics_descriptor_count;
  size_t diagnostics_descriptor_capacity;
  size_t security_descriptor_count;
  size_t security_descriptor_capacity;
  size_t max_parameters;
  size_t parameter_name_buffer_bytes;
  size_t parameter_value_buffer_bytes;
} grcl_runtime_capability_record_t;

typedef struct grcl_runtime_capability_request {
  size_t struct_size;
  uint32_t abi_version;
  grcl_capability_scope_t scope;
  uint64_t target_runtime_id;
  uint32_t target_domain_id;
  uint32_t required_profile_id;
  uint32_t optional_profile_id;
  uint32_t required_runtime_class_id;
  uint32_t optional_runtime_class_id;
  uint32_t min_grcl_protocol_major;
  uint32_t min_grcl_protocol_minor;
  uint32_t max_grcl_protocol_major;
  uint32_t max_grcl_protocol_minor;
  uint32_t min_capability_schema_version;
  uint32_t required_graph_projection_modes;
  uint32_t optional_graph_projection_modes;
  grcl_graph_projection_mode_t preferred_graph_projection_mode;
  uint32_t required_summary_flags;
  uint32_t optional_summary_flags;
  size_t required_min_payload_bytes;
  size_t optional_min_payload_bytes;
  size_t required_min_entities;
  size_t optional_min_entities;
  size_t required_min_nodes;
  size_t optional_min_nodes;
  size_t required_min_endpoints;
  size_t optional_min_endpoints;
  size_t required_min_sessions;
  size_t optional_min_sessions;
  size_t required_min_channels;
  size_t optional_min_channels;
  size_t required_min_message_buffer_bytes;
  size_t optional_min_message_buffer_bytes;
  size_t required_transport_descriptor_count;
  size_t optional_transport_descriptor_count;
  size_t required_qos_descriptor_count;
  size_t optional_qos_descriptor_count;
  size_t required_encoding_descriptor_count;
  size_t optional_encoding_descriptor_count;
  size_t required_diagnostics_descriptor_count;
  size_t optional_diagnostics_descriptor_count;
  size_t required_security_descriptor_count;
  size_t optional_security_descriptor_count;
} grcl_runtime_capability_request_t;

typedef struct grcl_capability_negotiation_result {
  size_t struct_size;
  uint32_t abi_version;
  grcl_capability_negotiation_status_t status;
  uint32_t diagnostic_code;
  grcl_capability_scope_t scope;
  uint32_t result_category;
  uint64_t effective_runtime_id;
  uint32_t effective_domain_id;
  uint32_t effective_profile_id;
  uint32_t effective_runtime_class_id;
  uint32_t effective_grcl_protocol_major;
  uint32_t effective_grcl_protocol_minor;
  uint32_t effective_capability_schema_version;
  grcl_graph_projection_mode_t effective_graph_projection_mode;
  uint32_t effective_summary_flags;
  size_t effective_max_payload_bytes;
  size_t effective_max_entities;
  size_t effective_max_nodes;
  size_t effective_max_endpoints;
  size_t effective_max_sessions;
  size_t effective_max_channels;
  size_t effective_transport_descriptor_count;
  size_t effective_qos_descriptor_count;
  size_t effective_encoding_descriptor_count;
  size_t effective_diagnostics_descriptor_count;
  size_t effective_security_descriptor_count;
  size_t rejection_reason_count;
  size_t rejection_reason_capacity;
  uint32_t suggested_downgrade_present;
  grcl_capability_scope_t suggested_downgrade_scope;
  grcl_graph_projection_mode_t suggested_downgrade_graph_projection_mode;
  uint32_t suggested_downgrade_summary_flags;
  size_t suggested_downgrade_max_payload_bytes;
  size_t suggested_downgrade_transport_descriptor_count;
  size_t suggested_downgrade_qos_descriptor_count;
  size_t suggested_downgrade_encoding_descriptor_count;
  size_t suggested_downgrade_diagnostics_descriptor_count;
  size_t suggested_downgrade_security_descriptor_count;
} grcl_capability_negotiation_result_t;

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_CAPABILITY_H_ */
