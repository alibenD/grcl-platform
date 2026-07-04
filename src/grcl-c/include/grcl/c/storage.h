#ifndef GRCL_C_STORAGE_H_
#define GRCL_C_STORAGE_H_

#include <stddef.h>
#include <stdint.h>

#include <grcl/c/types.h>
#include <grcl/c/version.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum grcl_storage_region_kind {
  GRCL_STORAGE_REGION_KIND_UNKNOWN = 0x0000,
  GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT = 0x0001,
  GRCL_STORAGE_REGION_KIND_NODE_TABLE = 0x0002,
  GRCL_STORAGE_REGION_KIND_ENDPOINT_TABLE = 0x0003,
  GRCL_STORAGE_REGION_KIND_SESSION_TABLE = 0x0004,
  GRCL_STORAGE_REGION_KIND_CHANNEL_TABLE = 0x0005,
  GRCL_STORAGE_REGION_KIND_MESSAGE_BUFFER = 0x0006,
  GRCL_STORAGE_REGION_KIND_GRAPH_CACHE = 0x0007,
  GRCL_STORAGE_REGION_KIND_DIAGNOSTICS_BUFFER = 0x0008,
  GRCL_STORAGE_REGION_KIND_EXECUTOR_STATE = 0x0009
} grcl_storage_region_kind_t;

typedef enum grcl_storage_flags {
  GRCL_STORAGE_FLAG_NONE = 0x00000000u,
  GRCL_STORAGE_FLAG_CALLER_PROVIDED_REGIONS = 0x00000001u,
  GRCL_STORAGE_FLAG_NO_HEAP_AFTER_INIT = 0x00000002u,
  GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES = 0x00000004u
} grcl_storage_flags_t;

struct grcl_storage_region {
  size_t struct_size;
  uint32_t abi_version;
  grcl_storage_region_kind_t kind;
  void * data;
  size_t bytes;
  size_t alignment;
  uint32_t flags;
};

struct grcl_storage {
  size_t struct_size;
  uint32_t abi_version;
  uint32_t flags;
  const grcl_storage_region_t * regions;
  size_t region_count;
  size_t runtime_object_bytes;
  size_t node_table_capacity;
  size_t endpoint_table_capacity;
  size_t session_table_capacity;
  size_t channel_table_capacity;
  size_t message_buffer_bytes;
  size_t graph_cache_bytes;
  size_t diagnostics_buffer_bytes;
  size_t executor_state_bytes;
};

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_STORAGE_H_ */
