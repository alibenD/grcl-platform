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

grcl_result_t grcl_subscription_create(
  grcl_node_t * node,
  const grcl_subscription_options_t * options,
  grcl_subscription_t ** subscription);

grcl_result_t grcl_subscription_destroy(grcl_subscription_t * subscription);

grcl_result_t grcl_subscription_get_endpoint(
  const grcl_subscription_t * subscription,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_service_create(
  grcl_node_t * node,
  const grcl_service_options_t * options,
  grcl_service_t ** service);

grcl_result_t grcl_service_destroy(grcl_service_t * service);

grcl_result_t grcl_service_get_endpoint(
  const grcl_service_t * service,
  grcl_endpoint_t ** endpoint);

grcl_result_t grcl_client_create(
  grcl_node_t * node,
  const grcl_client_options_t * options,
  grcl_client_t ** client);

grcl_result_t grcl_client_destroy(grcl_client_t * client);

grcl_result_t grcl_client_get_endpoint(
  const grcl_client_t * client,
  grcl_endpoint_t ** endpoint);

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

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_RUNTIME_H_ */
