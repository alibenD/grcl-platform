#ifndef GRCL_C_TYPES_H_
#define GRCL_C_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct grcl_runtime grcl_runtime_t;
typedef struct grcl_node grcl_node_t;
typedef struct grcl_endpoint grcl_endpoint_t;
typedef struct grcl_publisher grcl_publisher_t;
typedef struct grcl_subscription grcl_subscription_t;
typedef struct grcl_service grcl_service_t;
typedef struct grcl_client grcl_client_t;
typedef struct grcl_executor grcl_executor_t;
typedef struct grcl_type_support grcl_type_support_t;
typedef struct grcl_backend grcl_backend_t;
typedef struct grcl_transport grcl_transport_t;
typedef struct grcl_allocator grcl_allocator_t;
typedef struct grcl_allocator_config grcl_allocator_config_t;
typedef struct grcl_storage grcl_storage_t;
typedef struct grcl_storage_region grcl_storage_region_t;
typedef struct grcl_diagnostic_record grcl_diagnostic_record_t;

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_TYPES_H_ */
