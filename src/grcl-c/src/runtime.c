#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

typedef enum grcl_runtime_lifecycle_state {
  GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED = 1,
  GRCL_RUNTIME_LIFECYCLE_STATE_STARTED = 2,
  GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED = 3
} grcl_runtime_lifecycle_state_t;

typedef enum grcl_runtime_storage_ownership {
  GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP = 1,
  GRCL_RUNTIME_STORAGE_OWNERSHIP_CALLER_STORAGE = 2
} grcl_runtime_storage_ownership_t;

struct grcl_runtime {
  grcl_runtime_lifecycle_state_t state;
  grcl_runtime_storage_ownership_t ownership;
  const grcl_runtime_options_t * options;
  const grcl_storage_t * storage;
  const grcl_backend_descriptor_t * backend;
  grcl_backend_runtime_state_t * backend_state;
  int has_latest_diagnostic;
  grcl_diagnostic_record_t latest_diagnostic;
};

const grcl_backend_descriptor_t * grcl_null_native_test_backend_descriptor(void);

static grcl_runtime_t * grcl_runtime_allocate(void)
{
  return (grcl_runtime_t *)calloc(1u, sizeof(grcl_runtime_t));
}

static int grcl_storage_has_bounded_capacities(const grcl_storage_t * storage)
{
  return storage != NULL &&
    (storage->flags & GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES) != 0u;
}

static const grcl_storage_region_t * grcl_find_runtime_object_region(
  const grcl_storage_t * storage)
{
  if (storage == NULL || storage->regions == NULL ||
    storage->region_count == 0u ||
    storage->runtime_object_bytes < sizeof(grcl_runtime_t)) {
    return NULL;
  }

  for (size_t i = 0u; i < storage->region_count; ++i) {
    const grcl_storage_region_t * region = &storage->regions[i];
    if (region->kind != GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT) {
      continue;
    }
    if (region->data == NULL || region->bytes < sizeof(grcl_runtime_t)) {
      return NULL;
    }
    if (((uintptr_t)region->data % _Alignof(grcl_runtime_t)) != 0u) {
      return NULL;
    }

    return region;
  }

  return NULL;
}

static const grcl_backend_descriptor_t * grcl_runtime_default_backend(void)
{
  return grcl_null_native_test_backend_descriptor();
}

static void grcl_runtime_clear_latest_diagnostic(grcl_runtime_t * runtime)
{
  if (runtime == NULL) {
    return;
  }

  runtime->has_latest_diagnostic = 0;
  memset(&runtime->latest_diagnostic, 0, sizeof(runtime->latest_diagnostic));
}

static void grcl_runtime_record_diagnostic(
  grcl_runtime_t * runtime,
  grcl_diagnostic_code_t code,
  grcl_diagnostic_category_t category,
  grcl_diagnostic_scope_t scope)
{
  if (runtime == NULL) {
    return;
  }

  memset(&runtime->latest_diagnostic, 0, sizeof(runtime->latest_diagnostic));
  runtime->latest_diagnostic.struct_size = sizeof(runtime->latest_diagnostic);
  runtime->latest_diagnostic.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  runtime->latest_diagnostic.code = code;
  runtime->latest_diagnostic.severity = GRCL_DIAGNOSTIC_SEVERITY_ERROR;
  runtime->latest_diagnostic.category = category;
  runtime->latest_diagnostic.scope = scope;
  runtime->latest_diagnostic.detail_flags = 0u;
  runtime->has_latest_diagnostic = 1;
}

static grcl_result_t grcl_runtime_create_backend(
  grcl_runtime_t * runtime,
  const grcl_runtime_options_t * options,
  const grcl_storage_t * storage)
{
  grcl_backend_runtime_context_t context;

  if (runtime == NULL || runtime->backend == NULL ||
    runtime->backend->ops == NULL ||
    runtime->backend->ops->create_runtime == NULL) {
    return GRCL_ERROR_BACKEND_FAILURE;
  }

  memset(&context, 0, sizeof(context));
  context.struct_size = sizeof(context);
  context.abi_version = runtime->backend->abi_version;
  context.runtime_options = options;
  context.storage = storage;
  context.allocator = options == NULL ? NULL : options->allocator;

  return runtime->backend->ops->create_runtime(&context, &runtime->backend_state);
}

static void grcl_runtime_destroy_backend(grcl_runtime_t * runtime)
{
  const grcl_backend_ops_t * ops = NULL;

  if (runtime == NULL || runtime->backend_state == NULL ||
    runtime->backend == NULL) {
    return;
  }

  ops = runtime->backend->ops;
  if (ops != NULL && ops->destroy_runtime != NULL) {
    (void)ops->destroy_runtime(runtime->backend_state);
  }
  runtime->backend_state = NULL;
}

grcl_result_t grcl_runtime_create(
  const grcl_runtime_options_t * options,
  grcl_runtime_t ** runtime)
{
  if (runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *runtime = grcl_runtime_allocate();
  if (*runtime == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  (*runtime)->state = GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED;
  (*runtime)->ownership = GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP;
  (*runtime)->options = options;
  (*runtime)->storage = NULL;
  (*runtime)->backend = grcl_runtime_default_backend();
  grcl_runtime_clear_latest_diagnostic(*runtime);

  grcl_result_t backend_result =
    grcl_runtime_create_backend(*runtime, options, NULL);
  if (backend_result != GRCL_OK) {
    free(*runtime);
    *runtime = NULL;
    return backend_result;
  }

  return GRCL_OK;
}

grcl_result_t grcl_runtime_init_with_storage(
  const grcl_runtime_options_t * options,
  const grcl_storage_t * storage,
  grcl_runtime_t ** runtime)
{
  const grcl_storage_region_t * runtime_region = NULL;

  if (runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *runtime = NULL;
  if (!grcl_storage_has_bounded_capacities(storage)) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  runtime_region = grcl_find_runtime_object_region(storage);
  if (runtime_region == NULL) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  *runtime = (grcl_runtime_t *)runtime_region->data;
  memset(*runtime, 0, sizeof(**runtime));
  (*runtime)->state = GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED;
  (*runtime)->ownership = GRCL_RUNTIME_STORAGE_OWNERSHIP_CALLER_STORAGE;
  (*runtime)->options = options;
  (*runtime)->storage = storage;
  (*runtime)->backend = grcl_runtime_default_backend();
  grcl_runtime_clear_latest_diagnostic(*runtime);

  grcl_result_t backend_result =
    grcl_runtime_create_backend(*runtime, options, storage);
  if (backend_result != GRCL_OK) {
    memset(*runtime, 0, sizeof(**runtime));
    *runtime = NULL;
    return backend_result;
  }

  return GRCL_OK;
}

grcl_result_t grcl_runtime_start(grcl_runtime_t * runtime)
{
  if (runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED &&
    runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BAD_STATE,
      GRCL_DIAGNOSTIC_CATEGORY_CALLER_ERROR,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return GRCL_ERROR_BAD_STATE;
  }

  if (runtime->backend == NULL || runtime->backend->ops == NULL ||
    runtime->backend->ops->start_runtime == NULL) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BACKEND_FAILURE,
      GRCL_DIAGNOSTIC_CATEGORY_RUNTIME,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return GRCL_ERROR_BACKEND_FAILURE;
  }

  grcl_result_t backend_result =
    runtime->backend->ops->start_runtime(runtime->backend_state);
  if (backend_result != GRCL_OK) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BACKEND_FAILURE,
      GRCL_DIAGNOSTIC_CATEGORY_RUNTIME,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return backend_result;
  }

  runtime->state = GRCL_RUNTIME_LIFECYCLE_STATE_STARTED;
  return GRCL_OK;
}

grcl_result_t grcl_runtime_stop(grcl_runtime_t * runtime)
{
  if (runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BAD_STATE,
      GRCL_DIAGNOSTIC_CATEGORY_CALLER_ERROR,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return GRCL_ERROR_BAD_STATE;
  }

  if (runtime->backend == NULL || runtime->backend->ops == NULL ||
    runtime->backend->ops->stop_runtime == NULL) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BACKEND_FAILURE,
      GRCL_DIAGNOSTIC_CATEGORY_RUNTIME,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return GRCL_ERROR_BACKEND_FAILURE;
  }

  grcl_result_t backend_result =
    runtime->backend->ops->stop_runtime(runtime->backend_state);
  if (backend_result != GRCL_OK) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BACKEND_FAILURE,
      GRCL_DIAGNOSTIC_CATEGORY_RUNTIME,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return backend_result;
  }

  runtime->state = GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED;
  return GRCL_OK;
}

grcl_result_t grcl_runtime_destroy(grcl_runtime_t * runtime)
{
  if (runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    grcl_runtime_record_diagnostic(
      runtime,
      GRCL_DIAGNOSTIC_CODE_BAD_STATE,
      GRCL_DIAGNOSTIC_CATEGORY_CALLER_ERROR,
      GRCL_DIAGNOSTIC_SCOPE_RUNTIME);
    return GRCL_ERROR_BAD_STATE;
  }

  grcl_runtime_destroy_backend(runtime);

  if (runtime->ownership == GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP) {
    free(runtime);
  } else {
    memset(runtime, 0, sizeof(*runtime));
  }

  return GRCL_OK;
}

grcl_result_t grcl_runtime_get_capabilities(
  const grcl_runtime_t * runtime,
  grcl_runtime_capability_record_t * out_record)
{
  if (runtime == NULL || out_record == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->backend == NULL || runtime->backend->ops == NULL ||
    runtime->backend->ops->get_capabilities == NULL) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  return runtime->backend->ops->get_capabilities(
    runtime->backend_state,
    out_record);
}

grcl_result_t grcl_runtime_negotiate_capabilities(
  const grcl_runtime_t * runtime,
  const grcl_runtime_capability_request_t * request,
  grcl_capability_negotiation_result_t * out_result)
{
  if (runtime == NULL || request == NULL || out_result == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->backend == NULL || runtime->backend->ops == NULL ||
    runtime->backend->ops->negotiate_capabilities == NULL) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  return runtime->backend->ops->negotiate_capabilities(
    runtime->backend_state,
    request,
    out_result);
}

grcl_result_t grcl_runtime_get_diagnostics(
  const grcl_runtime_t * runtime,
  grcl_diagnostic_record_t * out_records,
  size_t record_capacity,
  size_t * out_record_count)
{
  size_t backend_count = 0u;
  grcl_result_t backend_result = GRCL_OK;

  if (runtime == NULL || out_record_count == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (record_capacity > 0u && out_records == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  if (runtime->backend == NULL || runtime->backend->ops == NULL ||
    runtime->backend->ops->get_diagnostics == NULL) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  backend_result = runtime->backend->ops->get_diagnostics(
    runtime->backend_state,
    out_records,
    record_capacity,
    &backend_count);
  if (backend_result != GRCL_OK) {
    return backend_result;
  }

  if (backend_count > 0u) {
    *out_record_count = backend_count;
    return GRCL_OK;
  }

  if (!runtime->has_latest_diagnostic) {
    *out_record_count = 0u;
    return GRCL_OK;
  }

  *out_record_count = 1u;
  if (record_capacity > 0u) {
    out_records[0u] = runtime->latest_diagnostic;
  }

  return GRCL_OK;
}
