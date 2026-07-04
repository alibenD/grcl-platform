#ifndef GRCL_C_VERSION_H_
#define GRCL_C_VERSION_H_

#define GRCL_C_ABI_VERSION_MAJOR 0u
#define GRCL_C_ABI_VERSION_MINOR 1u
#define GRCL_C_ABI_VERSION_PATCH 0u

#define GRCL_C_ABI_VERSION_PACK(major, minor, patch) \
  ((((major) & 0xffu) << 24) | (((minor) & 0xffu) << 16) | ((patch) & 0xffffu))

#define GRCL_C_ABI_VERSION_CURRENT \
  GRCL_C_ABI_VERSION_PACK( \
    GRCL_C_ABI_VERSION_MAJOR, \
    GRCL_C_ABI_VERSION_MINOR, \
    GRCL_C_ABI_VERSION_PATCH)

#endif  /* GRCL_C_VERSION_H_ */
