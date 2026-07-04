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
};

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_BACKEND_H_ */
