#ifndef GRCL_C_RUNTIME_H_
#define GRCL_C_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <grcl/c/allocator.h>
#include <grcl/c/capability.h>
#include <grcl/c/diagnostics.h>
#include <grcl/c/result.h>
#include <grcl/c/storage.h>
#include <grcl/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum grcl_runtime_option_flags {
  GRCL_RUNTIME_OPTION_FLAG_NONE = 0x00000000u
} grcl_runtime_option_flags_t;

typedef enum grcl_node_option_flags {
  GRCL_NODE_OPTION_FLAG_NONE = 0x00000000u
} grcl_node_option_flags_t;

typedef enum grcl_endpoint_option_flags {
  GRCL_ENDPOINT_OPTION_FLAG_NONE = 0x00000000u
} grcl_endpoint_option_flags_t;

typedef enum grcl_executor_option_flags {
  GRCL_EXECUTOR_OPTION_FLAG_NONE = 0x00000000u
} grcl_executor_option_flags_t;

typedef uint64_t grcl_request_id_t;

typedef enum grcl_param_type {
  GRCL_PARAM_TYPE_UNKNOWN = 0x0000,
  GRCL_PARAM_TYPE_BOOL = 0x0001,
  GRCL_PARAM_TYPE_INT64 = 0x0002,
  GRCL_PARAM_TYPE_UINT64 = 0x0003,
  GRCL_PARAM_TYPE_FLOAT64 = 0x0004,
  GRCL_PARAM_TYPE_STRING = 0x0005,
  GRCL_PARAM_TYPE_BYTES = 0x0006
} grcl_param_type_t;

typedef struct grcl_runtime_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * runtime_name;
  const char * profile_name;
  const grcl_allocator_config_t * allocator;
} grcl_runtime_options_t;

typedef struct grcl_node_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * node_name;
  const char * node_namespace;
  const grcl_allocator_config_t * allocator;
} grcl_node_options_t;

typedef struct grcl_publisher_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * topic_name;
  const grcl_type_support_t * type_support;
} grcl_publisher_options_t;

typedef struct grcl_subscription_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * topic_name;
  const grcl_type_support_t * type_support;
} grcl_subscription_options_t;

typedef struct grcl_service_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * service_name;
  const grcl_type_support_t * request_type_support;
  const grcl_type_support_t * response_type_support;
} grcl_service_options_t;

typedef struct grcl_client_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const char * service_name;
  const grcl_type_support_t * request_type_support;
  const grcl_type_support_t * response_type_support;
} grcl_client_options_t;

typedef struct grcl_executor_options {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const grcl_allocator_config_t * allocator;
  const grcl_storage_t * storage;
} grcl_executor_options_t;

typedef struct grcl_param_record {
  size_t struct_size;
  uint32_t abi_version;
  const char * name;
  grcl_param_type_t type;
  const void * value;
  size_t value_size;
} grcl_param_record_t;

grcl_result_t grcl_runtime_create(
  const grcl_runtime_options_t * options,
  grcl_runtime_t ** runtime);

grcl_result_t grcl_runtime_init_with_storage(
  const grcl_runtime_options_t * options,
  const grcl_storage_t * storage,
  grcl_runtime_t ** runtime);

grcl_result_t grcl_runtime_start(grcl_runtime_t * runtime);
grcl_result_t grcl_runtime_stop(grcl_runtime_t * runtime);
grcl_result_t grcl_runtime_destroy(grcl_runtime_t * runtime);

grcl_result_t grcl_node_create(
  grcl_runtime_t * runtime,
  const grcl_node_options_t * options,
  grcl_node_t ** node);

grcl_result_t grcl_node_destroy(grcl_node_t * node);

grcl_result_t grcl_node_get_runtime(
  const grcl_node_t * node,
  grcl_runtime_t ** runtime);

grcl_result_t grcl_publisher_create(
  grcl_node_t * node,
  const grcl_publisher_options_t * options,
  grcl_publisher_t ** publisher);

grcl_result_t grcl_publisher_destroy(grcl_publisher_t * publisher);

grcl_result_t grcl_publisher_get_endpoint(
  const grcl_publisher_t * publisher,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_publisher_publish_bytes(
  grcl_publisher_t * publisher,
  const void * payload,
  size_t payload_size);

grcl_result_t grcl_subscription_create(
  grcl_node_t * node,
  const grcl_subscription_options_t * options,
  grcl_subscription_t ** subscription);

grcl_result_t grcl_subscription_destroy(grcl_subscription_t * subscription);

grcl_result_t grcl_subscription_get_endpoint(
  const grcl_subscription_t * subscription,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_subscription_take_bytes(
  grcl_subscription_t * subscription,
  void * out_payload,
  size_t payload_capacity,
  size_t * out_payload_size);

grcl_result_t grcl_service_create(
  grcl_node_t * node,
  const grcl_service_options_t * options,
  grcl_service_t ** service);

grcl_result_t grcl_service_destroy(grcl_service_t * service);

grcl_result_t grcl_service_get_endpoint(
  const grcl_service_t * service,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_service_take_request_bytes(
  grcl_service_t * service,
  void * out_request_payload,
  size_t request_payload_capacity,
  size_t * out_request_payload_size,
  grcl_request_id_t * out_request_id);

grcl_result_t grcl_service_send_response_bytes(
  grcl_service_t * service,
  grcl_request_id_t request_id,
  const void * response_payload,
  size_t response_payload_size);

grcl_result_t grcl_client_create(
  grcl_node_t * node,
  const grcl_client_options_t * options,
  grcl_client_t ** client);

grcl_result_t grcl_client_destroy(grcl_client_t * client);

grcl_result_t grcl_client_get_endpoint(
  const grcl_client_t * client,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_client_send_request_bytes(
  grcl_client_t * client,
  const void * request_payload,
  size_t request_payload_size,
  grcl_request_id_t * out_request_id);

grcl_result_t grcl_client_take_response_bytes(
  grcl_client_t * client,
  grcl_request_id_t request_id,
  void * out_response_payload,
  size_t response_payload_capacity,
  size_t * out_response_payload_size);

grcl_result_t grcl_executor_create(
  grcl_runtime_t * runtime,
  const grcl_executor_options_t * options,
  grcl_executor_t ** executor);

grcl_result_t grcl_executor_destroy(grcl_executor_t * executor);

grcl_result_t grcl_executor_add_node(
  grcl_executor_t * executor,
  grcl_node_t * node);

grcl_result_t grcl_executor_remove_node(
  grcl_executor_t * executor,
  grcl_node_t * node);

grcl_result_t grcl_executor_spin_once(
  grcl_executor_t * executor,
  uint64_t timeout_ns);

grcl_result_t grcl_runtime_get_capabilities(
  const grcl_runtime_t * runtime,
  grcl_runtime_capability_record_t * out_record);

grcl_result_t grcl_runtime_negotiate_capabilities(
  const grcl_runtime_t * runtime,
  const grcl_runtime_capability_request_t * request,
  grcl_capability_negotiation_result_t * out_result);

grcl_result_t grcl_runtime_get_diagnostics(
  const grcl_runtime_t * runtime,
  grcl_diagnostic_record_t * out_records,
  size_t record_capacity,
  size_t * out_record_count);

grcl_result_t grcl_runtime_param_set(
  grcl_runtime_t * runtime,
  const grcl_param_record_t * param);

grcl_result_t grcl_runtime_param_get(
  grcl_runtime_t * runtime,
  const char * name,
  grcl_param_record_t * out_param,
  void * value_buffer,
  size_t value_buffer_capacity,
  size_t * out_value_size);

grcl_result_t grcl_runtime_param_list(
  grcl_runtime_t * runtime,
  char * out_names,
  size_t names_capacity,
  size_t * out_names_size,
  size_t * out_param_count);

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_RUNTIME_H_ */
