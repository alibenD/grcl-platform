#ifndef GRCL_C_ALLOCATOR_H_
#define GRCL_C_ALLOCATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <grcl/c/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * (*grcl_allocate_fn)(
  void * user_data,
  size_t size,
  size_t alignment);

typedef void (*grcl_deallocate_fn)(
  void * user_data,
  void * pointer,
  size_t size,
  size_t alignment);

typedef enum grcl_allocator_flags {
  GRCL_ALLOCATOR_FLAG_NONE = 0x00000000u,
  GRCL_ALLOCATOR_FLAG_CALLER_SYNCHRONIZED = 0x00000001u,
  GRCL_ALLOCATOR_FLAG_NO_HEAP_AFTER_INIT = 0x00000002u,
  GRCL_ALLOCATOR_FLAG_BOUNDED_ALLOCATIONS = 0x00000004u
} grcl_allocator_flags_t;

struct grcl_allocator_config {
  size_t struct_size;
  uint32_t abi_version;
  void * user_data;
  grcl_allocate_fn allocate;
  grcl_deallocate_fn deallocate;
  uint32_t flags;
  size_t max_allocation_bytes;
};

#ifdef __cplusplus
}
#endif

#endif  /* GRCL_C_ALLOCATOR_H_ */
