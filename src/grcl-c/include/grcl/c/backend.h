#ifndef GRCL_C_BACKEND_H_
#define GRCL_C_BACKEND_H_

#include <stddef.h>
#include <stdint.h>

#include <grcl/c/allocator.h>
#include <grcl/c/capability.h>
#include <grcl/c/diagnostics.h>
#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum grcl_backend_family_id {
  GRCL_BACKEND_FAMILY_ID_UNKNOWN = 0x0000u,
  GRCL_BACKEND_FAMILY_ID_NULL_NATIVE_TEST = 0x0001u,
  GRCL_BACKEND_FAMILY_ID_NATIVE = 0x0002u,
  GRCL_BACKEND_FAMILY_ID_ROS2 = 0x0003u,
  GRCL_BACKEND_FAMILY_ID_MCU = 0x0004u,
  GRCL_BACKEND_FAMILY_ID_GATEWAY = 0x0005u,
  GRCL_BACKEND_FAMILY_ID_SIMULATOR = 0x0006u
} grcl_backend_family_id_t;

typedef struct grcl_backend_runtime_context grcl_backend_runtime_context_t;
typedef struct grcl_backend_runtime_state grcl_backend_runtime_state_t;
typedef struct grcl_backend_node_state grcl_backend_node_state_t;
typedef struct grcl_backend_publisher_state grcl_backend_publisher_state_t;
typedef struct grcl_backend_subscription_state grcl_backend_subscription_state_t;
typedef struct grcl_backend_service_state grcl_backend_service_state_t;
typedef struct grcl_backend_client_state grcl_backend_client_state_t;
typedef struct grcl_backend_executor_state grcl_backend_executor_state_t;
typedef struct grcl_backend_descriptor grcl_backend_descriptor_t;
typedef struct grcl_backend_ops grcl_backend_ops_t;

struct grcl_backend_runtime_context {
  size_t struct_size;
  uint32_t abi_version;
  const grcl_runtime_options_t * runtime_options;
  const grcl_storage_t * storage;
  const grcl_allocator_config_t * allocator;
};

struct grcl_backend_descriptor {
  size_t struct_size;
  uint32_t abi_version;
  const char * backend_name;
  uint32_t backend_family_id;
  uint32_t backend_version_major;
  uint32_t backend_version_minor;
  uint32_t backend_version_patch;
  const grcl_backend_ops_t * ops;
};

struct grcl_backend_ops {
  size_t struct_size;
  uint32_t abi_version;
  grcl_result_t (*create_runtime)(
    const grcl_backend_runtime_context_t * context,
    grcl_backend_runtime_state_t ** out_backend_state);
  grcl_result_t (*start_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*stop_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*destroy_runtime)(
    grcl_backend_runtime_state_t * backend_state);
  grcl_result_t (*get_capabilities)(
    const grcl_backend_runtime_state_t * backend_state,
    grcl_runtime_capability_record_t * out_record);
  grcl_result_t (*negotiate_capabilities)(
    const grcl_backend_runtime_state_t * backend_state,
    const grcl_runtime_capability_request_t * request,
    grcl_capability_negotiation_result_t * out_result);
  grcl_result_t (*get_diagnostics)(
    const grcl_backend_runtime_state_t * backend_state,
    grcl_diagnostic_record_t * out_records,
    size_t record_capacity,
    size_t * out_record_count);
  grcl_result_t (*create_node)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_node_t * node,
    const grcl_node_options_t * options,
    grcl_backend_node_state_t ** out_backend_node);
  grcl_result_t (*destroy_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*create_publisher)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_publisher_t * publisher,
    const grcl_publisher_options_t * options,
    grcl_backend_publisher_state_t ** out_backend_publisher);
  grcl_result_t (*destroy_publisher)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_publisher_state_t * backend_publisher);
  grcl_result_t (*create_subscription)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_subscription_t * subscription,
    const grcl_subscription_options_t * options,
    grcl_backend_subscription_state_t ** out_backend_subscription);
  grcl_result_t (*destroy_subscription)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_subscription_state_t * backend_subscription);
  grcl_result_t (*publish_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_publisher_state_t * backend_publisher,
    const void * payload,
    size_t payload_size);
  grcl_result_t (*subscription_take_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_subscription_state_t * backend_subscription,
    void * out_payload,
    size_t payload_capacity,
    size_t * out_payload_size);
  grcl_result_t (*create_service)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_service_t * service,
    const grcl_service_options_t * options,
    grcl_backend_service_state_t ** out_backend_service);
  grcl_result_t (*destroy_service)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service);
  grcl_result_t (*create_client)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_node_state_t * backend_node,
    const grcl_client_t * client,
    const grcl_client_options_t * options,
    grcl_backend_client_state_t ** out_backend_client);
  grcl_result_t (*destroy_client)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client);
  grcl_result_t (*client_send_request_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client,
    const void * request_payload,
    size_t request_payload_size,
    grcl_request_id_t * out_request_id);
  grcl_result_t (*service_take_request_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service,
    void * out_request_payload,
    size_t request_payload_capacity,
    size_t * out_request_payload_size,
    grcl_request_id_t * out_request_id);
  grcl_result_t (*service_send_response_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_service_state_t * backend_service,
    grcl_request_id_t request_id,
    const void * response_payload,
    size_t response_payload_size);
  grcl_result_t (*client_take_response_bytes)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_client_state_t * backend_client,
    grcl_request_id_t request_id,
    void * out_response_payload,
    size_t response_payload_capacity,
    size_t * out_response_payload_size);
  grcl_result_t (*create_executor)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_executor_t * executor,
    const grcl_executor_options_t * options,
    grcl_backend_executor_state_t ** out_backend_executor);
  grcl_result_t (*destroy_executor)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor);
  grcl_result_t (*executor_add_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*executor_remove_node)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    grcl_backend_node_state_t * backend_node);
  grcl_result_t (*executor_spin_once)(
    grcl_backend_runtime_state_t * backend_state,
    grcl_backend_executor_state_t * backend_executor,
    uint64_t timeout_ns);
  grcl_result_t (*runtime_param_set)(
    grcl_backend_runtime_state_t * backend_state,
    const grcl_param_record_t * param);
  grcl_result_t (*runtime_param_get)(
    grcl_backend_runtime_state_t * backend_state,
    const char * name,
    grcl_param_record_t * out_param,
    void * value_buffer,
    size_t value_buffer_capacity,
    size_t * out_value_size);
  grcl_result_t (*runtime_param_list)(
    grcl_backend_runtime_state_t * backend_state,
    char * out_names,
    size_t names_capacity,
    size_t * out_names_size,
    size_t * out_param_count);
};

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_BACKEND_H_ */
