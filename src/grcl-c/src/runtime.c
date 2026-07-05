#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

#define GRCL_DEFAULT_NODE_CAPACITY 8u
#define GRCL_DEFAULT_ENDPOINT_CAPACITY 32u
#define GRCL_DEFAULT_EXECUTOR_CAPACITY 4u

typedef enum grcl_runtime_lifecycle_state {
  GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED = 1,
  GRCL_RUNTIME_LIFECYCLE_STATE_STARTED = 2,
  GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED = 3
} grcl_runtime_lifecycle_state_t;

typedef enum grcl_runtime_storage_ownership {
  GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP = 1,
  GRCL_RUNTIME_STORAGE_OWNERSHIP_CALLER_STORAGE = 2
} grcl_runtime_storage_ownership_t;

typedef enum grcl_endpoint_kind {
  GRCL_ENDPOINT_KIND_PUBLISHER = 1,
  GRCL_ENDPOINT_KIND_SUBSCRIPTION = 2,
  GRCL_ENDPOINT_KIND_SERVICE = 3,
  GRCL_ENDPOINT_KIND_CLIENT = 4
} grcl_endpoint_kind_t;

struct grcl_endpoint {
  grcl_runtime_t * runtime;
  grcl_node_t * node;
  void * owner_object;
  grcl_endpoint_kind_t kind;
  int destroyed;
  struct grcl_endpoint * next_all;
};

struct grcl_publisher {
  grcl_node_t * node;
  grcl_endpoint_t * endpoint;
  grcl_backend_publisher_state_t * backend_state;
  int destroyed;
  struct grcl_publisher * next_all;
  struct grcl_publisher * next_on_node;
};

struct grcl_subscription {
  grcl_node_t * node;
  grcl_endpoint_t * endpoint;
  grcl_backend_subscription_state_t * backend_state;
  int destroyed;
  struct grcl_subscription * next_all;
  struct grcl_subscription * next_on_node;
};

struct grcl_service {
  grcl_node_t * node;
  grcl_endpoint_t * endpoint;
  grcl_backend_service_state_t * backend_state;
  int destroyed;
  struct grcl_service * next_all;
  struct grcl_service * next_on_node;
};

struct grcl_client {
  grcl_node_t * node;
  grcl_endpoint_t * endpoint;
  grcl_backend_client_state_t * backend_state;
  int destroyed;
  struct grcl_client * next_all;
  struct grcl_client * next_on_node;
};

struct grcl_node {
  grcl_runtime_t * runtime;
  grcl_backend_node_state_t * backend_state;
  int destroyed;
  struct grcl_node * next_all;
  grcl_publisher_t * publishers;
  grcl_subscription_t * subscriptions;
  grcl_service_t * services;
  grcl_client_t * clients;
};

struct grcl_executor {
  grcl_runtime_t * runtime;
  grcl_backend_executor_state_t * backend_state;
  grcl_node_t ** members;
  size_t member_capacity;
  size_t member_count;
  int destroyed;
  struct grcl_executor * next_all;
};

struct grcl_runtime {
  grcl_runtime_lifecycle_state_t state;
  grcl_runtime_storage_ownership_t ownership;
  const grcl_runtime_options_t * options;
  const grcl_storage_t * storage;
  const grcl_backend_descriptor_t * backend;
  grcl_backend_runtime_state_t * backend_state;
  int has_latest_diagnostic;
  grcl_diagnostic_record_t latest_diagnostic;
  size_t node_capacity;
  size_t endpoint_capacity;
  size_t executor_capacity;
  grcl_node_t ** node_table;
  grcl_endpoint_t ** endpoint_table;
  grcl_executor_t ** executor_table;
  grcl_node_t * all_nodes;
  grcl_endpoint_t * all_endpoints;
  grcl_publisher_t * all_publishers;
  grcl_subscription_t * all_subscriptions;
  grcl_service_t * all_services;
  grcl_client_t * all_clients;
  grcl_executor_t * all_executors;
};

const grcl_backend_descriptor_t * grcl_null_native_test_backend_descriptor(void);
const grcl_backend_descriptor_t * grcl_native_inprocess_backend_descriptor(void);

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

static const grcl_backend_descriptor_t * grcl_runtime_select_backend(
  const grcl_runtime_options_t * options)
{
  if (options != NULL && options->profile_name != NULL &&
    strcmp(options->profile_name, "native-inprocess") == 0) {
    return grcl_native_inprocess_backend_descriptor();
  }

  return grcl_runtime_default_backend();
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

static int grcl_runtime_is_object_mutation_allowed(const grcl_runtime_t * runtime)
{
  return runtime != NULL &&
    (runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED ||
    runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED);
}

static int grcl_runtime_is_live(const grcl_runtime_t * runtime)
{
  return runtime != NULL &&
    (runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_INITIALIZED ||
    runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_STARTED ||
    runtime->state == GRCL_RUNTIME_LIFECYCLE_STATE_STOPPED);
}

static int grcl_backend_field_available(
  const grcl_backend_ops_t * ops,
  size_t field_offset,
  size_t field_size,
  const void * field_value)
{
  return ops != NULL &&
    ops->struct_size >= field_offset + field_size &&
    field_value != NULL;
}

static int grcl_runtime_prepare_tables(
  grcl_runtime_t * runtime,
  size_t node_capacity,
  size_t endpoint_capacity,
  size_t executor_capacity)
{
  runtime->node_capacity = node_capacity;
  runtime->endpoint_capacity = endpoint_capacity;
  runtime->executor_capacity = executor_capacity;

  runtime->node_table = node_capacity == 0u ? NULL :
    (grcl_node_t **)calloc(node_capacity, sizeof(grcl_node_t *));
  runtime->endpoint_table = endpoint_capacity == 0u ? NULL :
    (grcl_endpoint_t **)calloc(endpoint_capacity, sizeof(grcl_endpoint_t *));
  runtime->executor_table = executor_capacity == 0u ? NULL :
    (grcl_executor_t **)calloc(executor_capacity, sizeof(grcl_executor_t *));

  if ((node_capacity > 0u && runtime->node_table == NULL) ||
    (endpoint_capacity > 0u && runtime->endpoint_table == NULL) ||
    (executor_capacity > 0u && runtime->executor_table == NULL)) {
    free(runtime->node_table);
    free(runtime->endpoint_table);
    free(runtime->executor_table);
    runtime->node_table = NULL;
    runtime->endpoint_table = NULL;
    runtime->executor_table = NULL;
    return 0;
  }

  return 1;
}

static void grcl_runtime_release_tables(grcl_runtime_t * runtime)
{
  if (runtime == NULL) {
    return;
  }

  free(runtime->node_table);
  free(runtime->endpoint_table);
  free(runtime->executor_table);
  runtime->node_table = NULL;
  runtime->endpoint_table = NULL;
  runtime->executor_table = NULL;
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

static size_t grcl_runtime_live_slot_count(void ** table, size_t capacity)
{
  size_t count = 0u;

  for (size_t i = 0u; i < capacity; ++i) {
    if (table[i] != NULL) {
      ++count;
    }
  }

  return count;
}

static grcl_result_t grcl_runtime_assign_slot(
  void ** table,
  size_t capacity,
  void * value)
{
  if (capacity == 0u || table == NULL) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  for (size_t i = 0u; i < capacity; ++i) {
    if (table[i] == NULL) {
      table[i] = value;
      return GRCL_OK;
    }
  }

  return GRCL_ERROR_CAPACITY_EXCEEDED;
}

static void grcl_runtime_release_slot(void ** table, size_t capacity, void * value)
{
  if (table == NULL || value == NULL) {
    return;
  }

  for (size_t i = 0u; i < capacity; ++i) {
    if (table[i] == value) {
      table[i] = NULL;
      return;
    }
  }
}

static int grcl_required_name_valid(const char * name)
{
  return name != NULL && name[0] != '\0';
}

static int grcl_type_support_valid(const grcl_type_support_t * type_support)
{
  return type_support != NULL;
}

static int grcl_struct_size_at_least(size_t actual, size_t required)
{
  return actual >= required;
}

static void grcl_executor_detach_node(grcl_executor_t * executor, grcl_node_t * node)
{
  if (executor == NULL || node == NULL || executor->members == NULL) {
    return;
  }

  for (size_t i = 0u; i < executor->member_count; ++i) {
    if (executor->members[i] == node) {
      for (size_t j = i + 1u; j < executor->member_count; ++j) {
        executor->members[j - 1u] = executor->members[j];
      }
      executor->members[executor->member_count - 1u] = NULL;
      --executor->member_count;
      return;
    }
  }
}

static void grcl_runtime_detach_node_from_executors(
  grcl_runtime_t * runtime,
  grcl_node_t * node)
{
  if (runtime == NULL || node == NULL) {
    return;
  }

  for (size_t i = 0u; i < runtime->executor_capacity; ++i) {
    grcl_executor_t * executor = runtime->executor_table == NULL ?
      NULL : runtime->executor_table[i];
    if (executor == NULL || executor->destroyed) {
      continue;
    }
    grcl_executor_detach_node(executor, node);
  }
}

static void grcl_runtime_cleanup_endpoint_slot(
  grcl_runtime_t * runtime,
  grcl_endpoint_t * endpoint)
{
  if (runtime == NULL || endpoint == NULL) {
    return;
  }

  grcl_runtime_release_slot(
    (void **)runtime->endpoint_table,
    runtime->endpoint_capacity,
    endpoint);
  endpoint->destroyed = 1;
}

static void grcl_runtime_unlink_endpoint_from_cleanup_list(
  grcl_runtime_t * runtime,
  grcl_endpoint_t * endpoint)
{
  grcl_endpoint_t ** current;

  if (runtime == NULL || endpoint == NULL) {
    return;
  }

  current = &runtime->all_endpoints;
  while (*current != NULL) {
    if (*current == endpoint) {
      *current = endpoint->next_all;
      return;
    }
    current = &(*current)->next_all;
  }
}

static void grcl_runtime_cleanup_node_slot(
  grcl_runtime_t * runtime,
  grcl_node_t * node)
{
  if (runtime == NULL || node == NULL) {
    return;
  }

  grcl_runtime_release_slot(
    (void **)runtime->node_table,
    runtime->node_capacity,
    node);
  node->destroyed = 1;
}

static void grcl_runtime_cleanup_executor_slot(
  grcl_runtime_t * runtime,
  grcl_executor_t * executor)
{
  if (runtime == NULL || executor == NULL) {
    return;
  }

  grcl_runtime_release_slot(
    (void **)runtime->executor_table,
    runtime->executor_capacity,
    executor);
  executor->destroyed = 1;
}

static grcl_result_t grcl_runtime_destroy_publisher_internal(grcl_publisher_t * publisher)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (publisher == NULL || publisher->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = publisher->node == NULL ? NULL : publisher->node->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_publisher),
      sizeof(ops->destroy_publisher),
      ops == NULL ? NULL : (const void *)ops->destroy_publisher)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (publisher->backend_state != NULL) {
    grcl_result_t result =
      ops->destroy_publisher(runtime->backend_state, publisher->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  publisher->backend_state = NULL;
  publisher->destroyed = 1;
  grcl_runtime_cleanup_endpoint_slot(runtime, publisher->endpoint);
  return GRCL_OK;
}

static grcl_result_t grcl_runtime_destroy_subscription_internal(
  grcl_subscription_t * subscription)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (subscription == NULL || subscription->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = subscription->node == NULL ? NULL : subscription->node->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_subscription),
      sizeof(ops->destroy_subscription),
      ops == NULL ? NULL : (const void *)ops->destroy_subscription)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (subscription->backend_state != NULL) {
    grcl_result_t result =
      ops->destroy_subscription(runtime->backend_state, subscription->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  subscription->backend_state = NULL;
  subscription->destroyed = 1;
  grcl_runtime_cleanup_endpoint_slot(runtime, subscription->endpoint);
  return GRCL_OK;
}

static grcl_result_t grcl_runtime_destroy_service_internal(grcl_service_t * service)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (service == NULL || service->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = service->node == NULL ? NULL : service->node->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_service),
      sizeof(ops->destroy_service),
      ops == NULL ? NULL : (const void *)ops->destroy_service)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (service->backend_state != NULL) {
    grcl_result_t result =
      ops->destroy_service(runtime->backend_state, service->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  service->backend_state = NULL;
  service->destroyed = 1;
  grcl_runtime_cleanup_endpoint_slot(runtime, service->endpoint);
  return GRCL_OK;
}

static grcl_result_t grcl_runtime_destroy_client_internal(grcl_client_t * client)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (client == NULL || client->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = client->node == NULL ? NULL : client->node->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_client),
      sizeof(ops->destroy_client),
      ops == NULL ? NULL : (const void *)ops->destroy_client)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (client->backend_state != NULL) {
    grcl_result_t result =
      ops->destroy_client(runtime->backend_state, client->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  client->backend_state = NULL;
  client->destroyed = 1;
  grcl_runtime_cleanup_endpoint_slot(runtime, client->endpoint);
  return GRCL_OK;
}

static grcl_result_t grcl_runtime_destroy_node_internal(grcl_node_t * node)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;
  grcl_result_t result;

  if (node == NULL || node->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = node->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  for (grcl_publisher_t * publisher = node->publishers;
    publisher != NULL; publisher = publisher->next_on_node) {
    if (!publisher->destroyed) {
      result = grcl_runtime_destroy_publisher_internal(publisher);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }
  for (grcl_subscription_t * subscription = node->subscriptions;
    subscription != NULL; subscription = subscription->next_on_node) {
    if (!subscription->destroyed) {
      result = grcl_runtime_destroy_subscription_internal(subscription);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }
  for (grcl_service_t * service = node->services;
    service != NULL; service = service->next_on_node) {
    if (!service->destroyed) {
      result = grcl_runtime_destroy_service_internal(service);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }
  for (grcl_client_t * client = node->clients;
    client != NULL; client = client->next_on_node) {
    if (!client->destroyed) {
      result = grcl_runtime_destroy_client_internal(client);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_node),
      sizeof(ops->destroy_node),
      ops == NULL ? NULL : (const void *)ops->destroy_node)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (node->backend_state != NULL) {
    result = ops->destroy_node(runtime->backend_state, node->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  node->backend_state = NULL;
  grcl_runtime_detach_node_from_executors(runtime, node);
  grcl_runtime_cleanup_node_slot(runtime, node);
  return GRCL_OK;
}

static grcl_result_t grcl_runtime_destroy_executor_internal(grcl_executor_t * executor)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;
  grcl_result_t result;

  if (executor == NULL || executor->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = executor->runtime;
  if (runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, destroy_executor),
      sizeof(ops->destroy_executor),
      ops == NULL ? NULL : (const void *)ops->destroy_executor)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  if (executor->backend_state != NULL) {
    result = ops->destroy_executor(runtime->backend_state, executor->backend_state);
    if (result != GRCL_OK) {
      return result;
    }
  }

  executor->backend_state = NULL;
  executor->member_count = 0u;
  grcl_runtime_cleanup_executor_slot(runtime, executor);
  return GRCL_OK;
}

static void grcl_runtime_free_cleanup_lists(grcl_runtime_t * runtime)
{
  grcl_node_t * node;
  grcl_endpoint_t * endpoint;
  grcl_publisher_t * publisher;
  grcl_subscription_t * subscription;
  grcl_service_t * service;
  grcl_client_t * client;
  grcl_executor_t * executor;

  if (runtime == NULL) {
    return;
  }

  publisher = runtime->all_publishers;
  while (publisher != NULL) {
    grcl_publisher_t * next = publisher->next_all;
    free(publisher);
    publisher = next;
  }
  subscription = runtime->all_subscriptions;
  while (subscription != NULL) {
    grcl_subscription_t * next = subscription->next_all;
    free(subscription);
    subscription = next;
  }
  service = runtime->all_services;
  while (service != NULL) {
    grcl_service_t * next = service->next_all;
    free(service);
    service = next;
  }
  client = runtime->all_clients;
  while (client != NULL) {
    grcl_client_t * next = client->next_all;
    free(client);
    client = next;
  }
  endpoint = runtime->all_endpoints;
  while (endpoint != NULL) {
    grcl_endpoint_t * next = endpoint->next_all;
    free(endpoint);
    endpoint = next;
  }
  executor = runtime->all_executors;
  while (executor != NULL) {
    grcl_executor_t * next = executor->next_all;
    free(executor->members);
    free(executor);
    executor = next;
  }
  node = runtime->all_nodes;
  while (node != NULL) {
    grcl_node_t * next = node->next_all;
    free(node);
    node = next;
  }
}

static size_t grcl_runtime_default_executor_capacity(const grcl_storage_t * storage)
{
  if (storage == NULL) {
    return GRCL_DEFAULT_EXECUTOR_CAPACITY;
  }

  return storage->executor_state_bytes == 0u ? 0u : 1u;
}

grcl_result_t grcl_runtime_create(
  const grcl_runtime_options_t * options,
  grcl_runtime_t ** runtime)
{
  grcl_result_t backend_result;

  if (runtime == NULL ||
    (options != NULL &&
    !grcl_struct_size_at_least(options->struct_size, sizeof(*options)))) {
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
  (*runtime)->backend = grcl_runtime_select_backend(options);
  grcl_runtime_clear_latest_diagnostic(*runtime);

  if (!grcl_runtime_prepare_tables(
      *runtime,
      GRCL_DEFAULT_NODE_CAPACITY,
      GRCL_DEFAULT_ENDPOINT_CAPACITY,
      GRCL_DEFAULT_EXECUTOR_CAPACITY)) {
    free(*runtime);
    *runtime = NULL;
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  backend_result = grcl_runtime_create_backend(*runtime, options, NULL);
  if (backend_result != GRCL_OK) {
    grcl_runtime_release_tables(*runtime);
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
  grcl_result_t backend_result;

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
  (*runtime)->backend = grcl_runtime_select_backend(options);
  grcl_runtime_clear_latest_diagnostic(*runtime);

  if (!grcl_runtime_prepare_tables(
      *runtime,
      storage->node_table_capacity,
      storage->endpoint_table_capacity,
      grcl_runtime_default_executor_capacity(storage))) {
    memset(*runtime, 0, sizeof(**runtime));
    *runtime = NULL;
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  backend_result = grcl_runtime_create_backend(*runtime, options, storage);
  if (backend_result != GRCL_OK) {
    grcl_runtime_release_tables(*runtime);
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

  for (size_t i = 0u; i < runtime->executor_capacity; ++i) {
    if (runtime->executor_table != NULL && runtime->executor_table[i] != NULL) {
      (void)grcl_runtime_destroy_executor_internal(runtime->executor_table[i]);
    }
  }
  for (size_t i = 0u; i < runtime->node_capacity; ++i) {
    if (runtime->node_table != NULL && runtime->node_table[i] != NULL) {
      (void)grcl_runtime_destroy_node_internal(runtime->node_table[i]);
    }
  }

  grcl_runtime_destroy_backend(runtime);
  grcl_runtime_free_cleanup_lists(runtime);
  grcl_runtime_release_tables(runtime);

  if (runtime->ownership == GRCL_RUNTIME_STORAGE_OWNERSHIP_HEAP) {
    free(runtime);
  } else {
    memset(runtime, 0, sizeof(*runtime));
  }

  return GRCL_OK;
}

grcl_result_t grcl_node_create(
  grcl_runtime_t * runtime,
  const grcl_node_options_t * options,
  grcl_node_t ** node)
{
  const grcl_backend_ops_t * ops;
  grcl_node_t * created;
  grcl_result_t result;

  if (runtime == NULL || options == NULL || node == NULL ||
    !grcl_struct_size_at_least(options->struct_size, sizeof(*options)) ||
    !grcl_required_name_valid(options->node_name)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!grcl_runtime_is_object_mutation_allowed(runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_node),
      sizeof(ops->create_node),
      ops == NULL ? NULL : (const void *)ops->create_node)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_node_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  result = grcl_runtime_assign_slot(
    (void **)runtime->node_table,
    runtime->node_capacity,
    created);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  created->runtime = runtime;
  created->next_all = runtime->all_nodes;
  runtime->all_nodes = created;

  result = ops->create_node(
    runtime->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_release_slot((void **)runtime->node_table, runtime->node_capacity, created);
    runtime->all_nodes = created->next_all;
    free(created);
    return result;
  }

  *node = created;
  return GRCL_OK;
}

grcl_result_t grcl_node_destroy(grcl_node_t * node)
{
  if (node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_node_internal(node);
}

grcl_result_t grcl_node_get_runtime(
  const grcl_node_t * node,
  grcl_runtime_t ** runtime)
{
  if (node == NULL || runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed || node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  *runtime = node->runtime;
  return GRCL_OK;
}

static grcl_result_t grcl_endpoint_create_common(
  grcl_node_t * node,
  grcl_endpoint_kind_t kind,
  void * owner_object,
  grcl_endpoint_t ** out_endpoint)
{
  grcl_endpoint_t * endpoint;
  grcl_result_t result;

  if (node == NULL || out_endpoint == NULL || node->runtime == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  endpoint = (grcl_endpoint_t *)calloc(1u, sizeof(*endpoint));
  if (endpoint == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  result = grcl_runtime_assign_slot(
    (void **)node->runtime->endpoint_table,
    node->runtime->endpoint_capacity,
    endpoint);
  if (result != GRCL_OK) {
    free(endpoint);
    return result;
  }

  endpoint->runtime = node->runtime;
  endpoint->node = node;
  endpoint->owner_object = owner_object;
  endpoint->kind = kind;
  endpoint->next_all = node->runtime->all_endpoints;
  node->runtime->all_endpoints = endpoint;

  *out_endpoint = endpoint;
  return GRCL_OK;
}

grcl_result_t grcl_publisher_create(
  grcl_node_t * node,
  const grcl_publisher_options_t * options,
  grcl_publisher_t ** publisher)
{
  const grcl_backend_ops_t * ops;
  grcl_publisher_t * created;
  grcl_result_t result;

  if (node == NULL || options == NULL || publisher == NULL ||
    !grcl_required_name_valid(options->topic_name) ||
    !grcl_type_support_valid(options->type_support)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed || !grcl_runtime_is_object_mutation_allowed(node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = node->runtime->backend == NULL ? NULL : node->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_publisher),
      sizeof(ops->create_publisher),
      ops == NULL ? NULL : (const void *)ops->create_publisher)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_publisher_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->node = node;
  result = grcl_endpoint_create_common(
    node,
    GRCL_ENDPOINT_KIND_PUBLISHER,
    created,
    &created->endpoint);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  result = ops->create_publisher(
    node->runtime->backend_state,
    node->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_unlink_endpoint_from_cleanup_list(node->runtime, created->endpoint);
    grcl_runtime_cleanup_endpoint_slot(node->runtime, created->endpoint);
    free(created->endpoint);
    free(created);
    return result;
  }

  created->next_all = node->runtime->all_publishers;
  node->runtime->all_publishers = created;
  created->next_on_node = node->publishers;
  node->publishers = created;
  *publisher = created;
  return GRCL_OK;
}

grcl_result_t grcl_publisher_destroy(grcl_publisher_t * publisher)
{
  if (publisher == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (publisher->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(publisher->node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_publisher_internal(publisher);
}

grcl_result_t grcl_publisher_get_endpoint(
  const grcl_publisher_t * publisher,
  grcl_endpoint_t ** endpoint)
{
  if (publisher == NULL || endpoint == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (publisher->destroyed || publisher->endpoint == NULL || publisher->endpoint->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  *endpoint = publisher->endpoint;
  return GRCL_OK;
}

grcl_result_t grcl_publisher_publish_bytes(
  grcl_publisher_t * publisher,
  const void * payload,
  size_t payload_size)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (publisher == NULL || (payload_size > 0u && payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (publisher->destroyed || publisher->node == NULL ||
    publisher->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = publisher->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, publish_bytes),
      sizeof(ops->publish_bytes),
      ops == NULL ? NULL : (const void *)ops->publish_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->publish_bytes(
    runtime->backend_state,
    publisher->backend_state,
    payload,
    payload_size);
}

grcl_result_t grcl_subscription_create(
  grcl_node_t * node,
  const grcl_subscription_options_t * options,
  grcl_subscription_t ** subscription)
{
  const grcl_backend_ops_t * ops;
  grcl_subscription_t * created;
  grcl_result_t result;

  if (node == NULL || options == NULL || subscription == NULL ||
    !grcl_required_name_valid(options->topic_name) ||
    !grcl_type_support_valid(options->type_support)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed || !grcl_runtime_is_object_mutation_allowed(node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = node->runtime->backend == NULL ? NULL : node->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_subscription),
      sizeof(ops->create_subscription),
      ops == NULL ? NULL : (const void *)ops->create_subscription)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_subscription_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->node = node;
  result = grcl_endpoint_create_common(
    node,
    GRCL_ENDPOINT_KIND_SUBSCRIPTION,
    created,
    &created->endpoint);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  result = ops->create_subscription(
    node->runtime->backend_state,
    node->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_unlink_endpoint_from_cleanup_list(node->runtime, created->endpoint);
    grcl_runtime_cleanup_endpoint_slot(node->runtime, created->endpoint);
    free(created->endpoint);
    free(created);
    return result;
  }

  created->next_all = node->runtime->all_subscriptions;
  node->runtime->all_subscriptions = created;
  created->next_on_node = node->subscriptions;
  node->subscriptions = created;
  *subscription = created;
  return GRCL_OK;
}

grcl_result_t grcl_subscription_destroy(grcl_subscription_t * subscription)
{
  if (subscription == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (subscription->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(subscription->node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_subscription_internal(subscription);
}

grcl_result_t grcl_subscription_get_endpoint(
  const grcl_subscription_t * subscription,
  grcl_endpoint_t ** endpoint)
{
  if (subscription == NULL || endpoint == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (subscription->destroyed || subscription->endpoint == NULL ||
    subscription->endpoint->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  *endpoint = subscription->endpoint;
  return GRCL_OK;
}

grcl_result_t grcl_subscription_take_bytes(
  grcl_subscription_t * subscription,
  void * out_payload,
  size_t payload_capacity,
  size_t * out_payload_size)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (subscription == NULL || out_payload_size == NULL ||
    (payload_capacity > 0u && out_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (subscription->destroyed || subscription->node == NULL ||
    subscription->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = subscription->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, subscription_take_bytes),
      sizeof(ops->subscription_take_bytes),
      ops == NULL ? NULL : (const void *)ops->subscription_take_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->subscription_take_bytes(
    runtime->backend_state,
    subscription->backend_state,
    out_payload,
    payload_capacity,
    out_payload_size);
}

grcl_result_t grcl_service_create(
  grcl_node_t * node,
  const grcl_service_options_t * options,
  grcl_service_t ** service)
{
  const grcl_backend_ops_t * ops;
  grcl_service_t * created;
  grcl_result_t result;

  if (node == NULL || options == NULL || service == NULL ||
    !grcl_required_name_valid(options->service_name) ||
    !grcl_type_support_valid(options->request_type_support) ||
    !grcl_type_support_valid(options->response_type_support)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed || !grcl_runtime_is_object_mutation_allowed(node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = node->runtime->backend == NULL ? NULL : node->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_service),
      sizeof(ops->create_service),
      ops == NULL ? NULL : (const void *)ops->create_service)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_service_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->node = node;
  result = grcl_endpoint_create_common(
    node,
    GRCL_ENDPOINT_KIND_SERVICE,
    created,
    &created->endpoint);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  result = ops->create_service(
    node->runtime->backend_state,
    node->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_unlink_endpoint_from_cleanup_list(node->runtime, created->endpoint);
    grcl_runtime_cleanup_endpoint_slot(node->runtime, created->endpoint);
    free(created->endpoint);
    free(created);
    return result;
  }

  created->next_all = node->runtime->all_services;
  node->runtime->all_services = created;
  created->next_on_node = node->services;
  node->services = created;
  *service = created;
  return GRCL_OK;
}

grcl_result_t grcl_service_destroy(grcl_service_t * service)
{
  if (service == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (service->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(service->node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_service_internal(service);
}

grcl_result_t grcl_service_get_endpoint(
  const grcl_service_t * service,
  grcl_endpoint_t ** endpoint)
{
  if (service == NULL || endpoint == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (service->destroyed || service->endpoint == NULL || service->endpoint->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  *endpoint = service->endpoint;
  return GRCL_OK;
}

grcl_result_t grcl_service_take_request_bytes(
  grcl_service_t * service,
  void * out_request_payload,
  size_t request_payload_capacity,
  size_t * out_request_payload_size,
  grcl_request_id_t * out_request_id)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (service == NULL || out_request_payload_size == NULL ||
    out_request_id == NULL ||
    (request_payload_capacity > 0u && out_request_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (service->destroyed || service->node == NULL ||
    service->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = service->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, service_take_request_bytes),
      sizeof(ops->service_take_request_bytes),
      ops == NULL ? NULL : (const void *)ops->service_take_request_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->service_take_request_bytes(
    runtime->backend_state,
    service->backend_state,
    out_request_payload,
    request_payload_capacity,
    out_request_payload_size,
    out_request_id);
}

grcl_result_t grcl_service_send_response_bytes(
  grcl_service_t * service,
  grcl_request_id_t request_id,
  const void * response_payload,
  size_t response_payload_size)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (service == NULL || (response_payload_size > 0u && response_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (service->destroyed || service->node == NULL ||
    service->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = service->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, service_send_response_bytes),
      sizeof(ops->service_send_response_bytes),
      ops == NULL ? NULL : (const void *)ops->service_send_response_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->service_send_response_bytes(
    runtime->backend_state,
    service->backend_state,
    request_id,
    response_payload,
    response_payload_size);
}

grcl_result_t grcl_client_create(
  grcl_node_t * node,
  const grcl_client_options_t * options,
  grcl_client_t ** client)
{
  const grcl_backend_ops_t * ops;
  grcl_client_t * created;
  grcl_result_t result;

  if (node == NULL || options == NULL || client == NULL ||
    !grcl_required_name_valid(options->service_name) ||
    !grcl_type_support_valid(options->request_type_support) ||
    !grcl_type_support_valid(options->response_type_support)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (node->destroyed || !grcl_runtime_is_object_mutation_allowed(node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = node->runtime->backend == NULL ? NULL : node->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_client),
      sizeof(ops->create_client),
      ops == NULL ? NULL : (const void *)ops->create_client)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_client_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->node = node;
  result = grcl_endpoint_create_common(
    node,
    GRCL_ENDPOINT_KIND_CLIENT,
    created,
    &created->endpoint);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  result = ops->create_client(
    node->runtime->backend_state,
    node->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_unlink_endpoint_from_cleanup_list(node->runtime, created->endpoint);
    grcl_runtime_cleanup_endpoint_slot(node->runtime, created->endpoint);
    free(created->endpoint);
    free(created);
    return result;
  }

  created->next_all = node->runtime->all_clients;
  node->runtime->all_clients = created;
  created->next_on_node = node->clients;
  node->clients = created;
  *client = created;
  return GRCL_OK;
}

grcl_result_t grcl_client_destroy(grcl_client_t * client)
{
  if (client == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (client->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(client->node->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_client_internal(client);
}

grcl_result_t grcl_client_get_endpoint(
  const grcl_client_t * client,
  grcl_endpoint_t ** endpoint)
{
  if (client == NULL || endpoint == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (client->destroyed || client->endpoint == NULL || client->endpoint->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }

  *endpoint = client->endpoint;
  return GRCL_OK;
}

grcl_result_t grcl_client_send_request_bytes(
  grcl_client_t * client,
  const void * request_payload,
  size_t request_payload_size,
  grcl_request_id_t * out_request_id)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (client == NULL || out_request_id == NULL ||
    (request_payload_size > 0u && request_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (client->destroyed || client->node == NULL ||
    client->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = client->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, client_send_request_bytes),
      sizeof(ops->client_send_request_bytes),
      ops == NULL ? NULL : (const void *)ops->client_send_request_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->client_send_request_bytes(
    runtime->backend_state,
    client->backend_state,
    request_payload,
    request_payload_size,
    out_request_id);
}

grcl_result_t grcl_client_take_response_bytes(
  grcl_client_t * client,
  grcl_request_id_t request_id,
  void * out_response_payload,
  size_t response_payload_capacity,
  size_t * out_response_payload_size)
{
  const grcl_backend_ops_t * ops;
  grcl_runtime_t * runtime;

  if (client == NULL || out_response_payload_size == NULL ||
    (response_payload_capacity > 0u && out_response_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (client->destroyed || client->node == NULL ||
    client->node->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  runtime = client->node->runtime;
  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, client_take_response_bytes),
      sizeof(ops->client_take_response_bytes),
      ops == NULL ? NULL : (const void *)ops->client_take_response_bytes)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->client_take_response_bytes(
    runtime->backend_state,
    client->backend_state,
    request_id,
    out_response_payload,
    response_payload_capacity,
    out_response_payload_size);
}

grcl_result_t grcl_executor_create(
  grcl_runtime_t * runtime,
  const grcl_executor_options_t * options,
  grcl_executor_t ** executor)
{
  const grcl_backend_ops_t * ops;
  grcl_executor_t * created;
  grcl_result_t result;

  if (runtime == NULL || options == NULL || executor == NULL ||
    !grcl_struct_size_at_least(options->struct_size, sizeof(*options))) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!grcl_runtime_is_object_mutation_allowed(runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, create_executor),
      sizeof(ops->create_executor),
      ops == NULL ? NULL : (const void *)ops->create_executor)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  created = (grcl_executor_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  result = grcl_runtime_assign_slot(
    (void **)runtime->executor_table,
    runtime->executor_capacity,
    created);
  if (result != GRCL_OK) {
    free(created);
    return result;
  }

  created->runtime = runtime;
  created->member_capacity = runtime->node_capacity;
  created->members = created->member_capacity == 0u ? NULL :
    (grcl_node_t **)calloc(created->member_capacity, sizeof(grcl_node_t *));
  if (created->member_capacity > 0u && created->members == NULL) {
    grcl_runtime_release_slot(
      (void **)runtime->executor_table,
      runtime->executor_capacity,
      created);
    free(created);
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->next_all = runtime->all_executors;
  runtime->all_executors = created;

  result = ops->create_executor(
    runtime->backend_state,
    created,
    options,
    &created->backend_state);
  if (result != GRCL_OK) {
    grcl_runtime_release_slot(
      (void **)runtime->executor_table,
      runtime->executor_capacity,
      created);
    runtime->all_executors = created->next_all;
    free(created->members);
    free(created);
    return result;
  }

  *executor = created;
  return GRCL_OK;
}

grcl_result_t grcl_executor_destroy(grcl_executor_t * executor)
{
  if (executor == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (executor->destroyed) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (!grcl_runtime_is_object_mutation_allowed(executor->runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_runtime_destroy_executor_internal(executor);
}

grcl_result_t grcl_executor_add_node(
  grcl_executor_t * executor,
  grcl_node_t * node)
{
  const grcl_backend_ops_t * ops;

  if (executor == NULL || node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (executor->destroyed || node->destroyed || executor->runtime != node->runtime) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = executor->runtime->backend == NULL ? NULL : executor->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, executor_add_node),
      sizeof(ops->executor_add_node),
      ops == NULL ? NULL : (const void *)ops->executor_add_node)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  for (size_t i = 0u; i < executor->member_count; ++i) {
    if (executor->members[i] == node) {
      return GRCL_OK;
    }
  }
  if (executor->member_count >= executor->member_capacity) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  grcl_result_t result = ops->executor_add_node(
    executor->runtime->backend_state,
    executor->backend_state,
    node->backend_state);
  if (result != GRCL_OK) {
    return result;
  }

  executor->members[executor->member_count++] = node;
  return GRCL_OK;
}

grcl_result_t grcl_executor_remove_node(
  grcl_executor_t * executor,
  grcl_node_t * node)
{
  const grcl_backend_ops_t * ops;
  int found = 0;

  if (executor == NULL || node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (executor->destroyed || node->destroyed || executor->runtime != node->runtime) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = executor->runtime->backend == NULL ? NULL : executor->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, executor_remove_node),
      sizeof(ops->executor_remove_node),
      ops == NULL ? NULL : (const void *)ops->executor_remove_node)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  for (size_t i = 0u; i < executor->member_count; ++i) {
    if (executor->members[i] == node) {
      found = 1;
      break;
    }
  }
  if (!found) {
    return GRCL_ERROR_NOT_FOUND;
  }

  grcl_result_t result = ops->executor_remove_node(
    executor->runtime->backend_state,
    executor->backend_state,
    node->backend_state);
  if (result != GRCL_OK) {
    return result;
  }

  grcl_executor_detach_node(executor, node);
  return GRCL_OK;
}

grcl_result_t grcl_executor_spin_once(
  grcl_executor_t * executor,
  uint64_t timeout_ns)
{
  const grcl_backend_ops_t * ops;

  if (executor == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (executor->destroyed || executor->runtime == NULL) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = executor->runtime->backend == NULL ? NULL : executor->runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, executor_spin_once),
      sizeof(ops->executor_spin_once),
      ops == NULL ? NULL : (const void *)ops->executor_spin_once)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }
  if (executor->runtime->state != GRCL_RUNTIME_LIFECYCLE_STATE_STARTED) {
    return GRCL_ERROR_BAD_STATE;
  }

  return ops->executor_spin_once(
    executor->runtime->backend_state,
    executor->backend_state,
    timeout_ns);
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

grcl_result_t grcl_runtime_param_set(
  grcl_runtime_t * runtime,
  const grcl_param_record_t * param)
{
  const grcl_backend_ops_t * ops;

  if (runtime == NULL || param == NULL ||
    !grcl_required_name_valid(param->name) ||
    param->type == GRCL_PARAM_TYPE_UNKNOWN ||
    (param->value_size > 0u && param->value == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!grcl_runtime_is_live(runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, runtime_param_set),
      sizeof(ops->runtime_param_set),
      ops == NULL ? NULL : (const void *)ops->runtime_param_set)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  return ops->runtime_param_set(runtime->backend_state, param);
}

grcl_result_t grcl_runtime_param_get(
  grcl_runtime_t * runtime,
  const char * name,
  grcl_param_record_t * out_param,
  void * value_buffer,
  size_t value_buffer_capacity,
  size_t * out_value_size)
{
  const grcl_backend_ops_t * ops;

  if (runtime == NULL || !grcl_required_name_valid(name) ||
    out_param == NULL || out_value_size == NULL ||
    (value_buffer_capacity > 0u && value_buffer == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!grcl_runtime_is_live(runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, runtime_param_get),
      sizeof(ops->runtime_param_get),
      ops == NULL ? NULL : (const void *)ops->runtime_param_get)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  return ops->runtime_param_get(
    runtime->backend_state,
    name,
    out_param,
    value_buffer,
    value_buffer_capacity,
    out_value_size);
}

grcl_result_t grcl_runtime_param_list(
  grcl_runtime_t * runtime,
  char * out_names,
  size_t names_capacity,
  size_t * out_names_size,
  size_t * out_param_count)
{
  const grcl_backend_ops_t * ops;

  if (runtime == NULL || out_names_size == NULL || out_param_count == NULL ||
    (names_capacity > 0u && out_names == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!grcl_runtime_is_live(runtime)) {
    return GRCL_ERROR_BAD_STATE;
  }

  ops = runtime->backend == NULL ? NULL : runtime->backend->ops;
  if (!grcl_backend_field_available(
      ops,
      offsetof(grcl_backend_ops_t, runtime_param_list),
      sizeof(ops->runtime_param_list),
      ops == NULL ? NULL : (const void *)ops->runtime_param_list)) {
    return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
  }

  return ops->runtime_param_list(
    runtime->backend_state,
    out_names,
    names_capacity,
    out_names_size,
    out_param_count);
}
