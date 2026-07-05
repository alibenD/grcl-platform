#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/version.h>

#define GRCL_INPROCESS_MAX_PAYLOAD_BYTES 1024u
#define GRCL_INPROCESS_QUEUE_CAPACITY 8u

typedef struct grcl_inprocess_message {
  size_t size;
  unsigned char data[GRCL_INPROCESS_MAX_PAYLOAD_BYTES];
} grcl_inprocess_message_t;

typedef struct grcl_inprocess_message_queue {
  grcl_inprocess_message_t messages[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t head;
  size_t count;
} grcl_inprocess_message_queue_t;

struct grcl_backend_node_state {
  struct grcl_backend_node_state * next;
};

struct grcl_backend_publisher_state {
  struct grcl_backend_runtime_state * runtime;
  grcl_backend_node_state_t * node;
  char * topic_name;
  uint64_t type_id;
  grcl_inprocess_message_queue_t pending;
  struct grcl_backend_publisher_state * next;
};

struct grcl_backend_subscription_state {
  struct grcl_backend_runtime_state * runtime;
  grcl_backend_node_state_t * node;
  char * topic_name;
  uint64_t type_id;
  grcl_inprocess_message_queue_t ready;
  struct grcl_backend_subscription_state * next;
};

struct grcl_backend_service_state {
  int placeholder;
};

struct grcl_backend_client_state {
  int placeholder;
};

struct grcl_backend_executor_state {
  grcl_backend_node_state_t * nodes[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t node_count;
};

struct grcl_backend_runtime_state {
  int started;
  grcl_backend_node_state_t * nodes;
  grcl_backend_publisher_state_t * publishers;
  grcl_backend_subscription_state_t * subscriptions;
};

static char * grcl_inprocess_copy_string(const char * value)
{
  size_t bytes;
  char * copy;

  if (value == NULL) {
    return NULL;
  }

  bytes = strlen(value) + 1u;
  copy = (char *)malloc(bytes);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, value, bytes);
  return copy;
}

static grcl_result_t grcl_inprocess_queue_push(
  grcl_inprocess_message_queue_t * queue,
  const void * payload,
  size_t payload_size)
{
  size_t index;

  if (queue == NULL || (payload_size > 0u && payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (payload_size > GRCL_INPROCESS_MAX_PAYLOAD_BYTES) {
    return GRCL_ERROR_PAYLOAD_TOO_LARGE;
  }
  if (queue->count >= GRCL_INPROCESS_QUEUE_CAPACITY) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  index = (queue->head + queue->count) % GRCL_INPROCESS_QUEUE_CAPACITY;
  queue->messages[index].size = payload_size;
  if (payload_size > 0u) {
    memcpy(queue->messages[index].data, payload, payload_size);
  }
  ++queue->count;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_queue_peek(
  grcl_inprocess_message_queue_t * queue,
  grcl_inprocess_message_t ** out_message)
{
  if (queue == NULL || out_message == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (queue->count == 0u) {
    return GRCL_ERROR_NO_DATA;
  }

  *out_message = &queue->messages[queue->head];
  return GRCL_OK;
}

static void grcl_inprocess_queue_pop(grcl_inprocess_message_queue_t * queue)
{
  if (queue == NULL || queue->count == 0u) {
    return;
  }

  queue->head = (queue->head + 1u) % GRCL_INPROCESS_QUEUE_CAPACITY;
  --queue->count;
}

static int grcl_inprocess_queue_has_capacity(
  const grcl_inprocess_message_queue_t * queue)
{
  return queue != NULL && queue->count < GRCL_INPROCESS_QUEUE_CAPACITY;
}

static int grcl_inprocess_endpoint_matches(
  const grcl_backend_publisher_state_t * publisher,
  const grcl_backend_subscription_state_t * subscription)
{
  return publisher != NULL && subscription != NULL &&
    publisher->type_id == subscription->type_id &&
    publisher->topic_name != NULL && subscription->topic_name != NULL &&
    strcmp(publisher->topic_name, subscription->topic_name) == 0;
}

static grcl_result_t grcl_inprocess_create_runtime(
  const grcl_backend_runtime_context_t * context,
  grcl_backend_runtime_state_t ** out_backend_state)
{
  grcl_backend_runtime_state_t * state;

  (void)context;

  if (out_backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_state = NULL;
  state = (grcl_backend_runtime_state_t *)calloc(1u, sizeof(*state));
  if (state == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  *out_backend_state = state;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_start_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  backend_state->started = 1;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_stop_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  backend_state->started = 0;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_destroy_runtime(
  grcl_backend_runtime_state_t * backend_state)
{
  if (backend_state == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  free(backend_state);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_get_capabilities(
  const grcl_backend_runtime_state_t * backend_state,
  grcl_runtime_capability_record_t * out_record)
{
  if (backend_state == NULL || out_record == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  memset(out_record, 0, sizeof(*out_record));
  out_record->struct_size = sizeof(*out_record);
  out_record->abi_version = GRCL_C_ABI_VERSION_CURRENT;
  out_record->runtime_id = 2u;
  out_record->boot_id = 1u;
  out_record->profile_id = 2u;
  out_record->runtime_class_id = GRCL_BACKEND_FAMILY_ID_NATIVE;
  out_record->implementation_id = GRCL_BACKEND_FAMILY_ID_NATIVE;
  out_record->implementation_version_minor = 1u;
  out_record->grcl_protocol_minor = 1u;
  out_record->capability_schema_version = 1u;
  out_record->supported_graph_projection_modes =
    GRCL_GRAPH_PROJECTION_MODE_FLAG_RUNTIME_ONLY;
  out_record->preferred_graph_projection_mode =
    GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY;
  out_record->default_graph_projection_mode =
    GRCL_GRAPH_PROJECTION_MODE_RUNTIME_ONLY;
  out_record->summary_flags =
    GRCL_CAPABILITY_SUMMARY_FLAG_BOUNDED_CAPACITIES |
    GRCL_CAPABILITY_SUMMARY_FLAG_DETERMINISTIC_DESTROY |
    GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS;
  out_record->max_payload_bytes = GRCL_INPROCESS_MAX_PAYLOAD_BYTES;
  out_record->max_entities = 32u;
  out_record->max_nodes = 8u;
  out_record->max_endpoints = 32u;
  out_record->max_publishers = 16u;
  out_record->max_subscriptions = 16u;
  out_record->message_buffer_bytes =
    GRCL_INPROCESS_MAX_PAYLOAD_BYTES * GRCL_INPROCESS_QUEUE_CAPACITY;
  out_record->executor_state_bytes = sizeof(grcl_backend_executor_state_t);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_negotiate_capabilities(
  const grcl_backend_runtime_state_t * backend_state,
  const grcl_runtime_capability_request_t * request,
  grcl_capability_negotiation_result_t * out_result)
{
  grcl_runtime_capability_record_t record;

  if (backend_state == NULL || request == NULL || out_result == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  memset(out_result, 0, sizeof(*out_result));
  out_result->struct_size = sizeof(*out_result);
  out_result->abi_version = GRCL_C_ABI_VERSION_CURRENT;
  out_result->status = GRCL_CAPABILITY_NEGOTIATION_STATUS_ACCEPTED;
  out_result->result_category = GRCL_RESULT_CATEGORY_SUCCESS;
  out_result->scope = request->scope == GRCL_CAPABILITY_SCOPE_UNKNOWN ?
    GRCL_CAPABILITY_SCOPE_SESSION : request->scope;

  (void)grcl_inprocess_get_capabilities(backend_state, &record);
  out_result->effective_runtime_id = record.runtime_id;
  out_result->effective_domain_id = record.domain_id;
  out_result->effective_profile_id = record.profile_id;
  out_result->effective_runtime_class_id = record.runtime_class_id;
  out_result->effective_grcl_protocol_minor = record.grcl_protocol_minor;
  out_result->effective_capability_schema_version = record.capability_schema_version;
  out_result->effective_graph_projection_mode = record.default_graph_projection_mode;
  out_result->effective_summary_flags = record.summary_flags;
  out_result->effective_max_payload_bytes = record.max_payload_bytes;
  out_result->effective_max_entities = record.max_entities;
  out_result->effective_max_nodes = record.max_nodes;
  out_result->effective_max_endpoints = record.max_endpoints;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_get_diagnostics(
  const grcl_backend_runtime_state_t * backend_state,
  grcl_diagnostic_record_t * out_records,
  size_t record_capacity,
  size_t * out_record_count)
{
  if (backend_state == NULL || out_record_count == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (record_capacity > 0u && out_records == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_record_count = 0u;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_create_node(
  grcl_backend_runtime_state_t * backend_state,
  const grcl_node_t * node,
  const grcl_node_options_t * options,
  grcl_backend_node_state_t ** out_backend_node)
{
  grcl_backend_node_state_t * created;

  (void)node;
  (void)options;

  if (backend_state == NULL || out_backend_node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_node = NULL;
  created = (grcl_backend_node_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->next = backend_state->nodes;
  backend_state->nodes = created;
  *out_backend_node = created;
  return GRCL_OK;
}

static void grcl_inprocess_remove_node_link(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * node)
{
  grcl_backend_node_state_t ** current;

  if (backend_state == NULL || node == NULL) {
    return;
  }

  current = &backend_state->nodes;
  while (*current != NULL) {
    if (*current == node) {
      *current = node->next;
      return;
    }
    current = &(*current)->next;
  }
}

static grcl_result_t grcl_inprocess_destroy_node(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node)
{
  if (backend_state == NULL || backend_node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_inprocess_remove_node_link(backend_state, backend_node);
  free(backend_node);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_create_publisher(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node,
  const grcl_publisher_t * publisher,
  const grcl_publisher_options_t * options,
  grcl_backend_publisher_state_t ** out_backend_publisher)
{
  grcl_backend_publisher_state_t * created;

  (void)publisher;

  if (backend_state == NULL || backend_node == NULL || options == NULL ||
    options->topic_name == NULL || options->type_support == NULL ||
    out_backend_publisher == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_publisher = NULL;
  created = (grcl_backend_publisher_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->topic_name = grcl_inprocess_copy_string(options->topic_name);
  if (created->topic_name == NULL) {
    free(created);
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->runtime = backend_state;
  created->node = backend_node;
  created->type_id = options->type_support->type_id;
  created->next = backend_state->publishers;
  backend_state->publishers = created;
  *out_backend_publisher = created;
  return GRCL_OK;
}

static void grcl_inprocess_remove_publisher_link(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_publisher_state_t * publisher)
{
  grcl_backend_publisher_state_t ** current;

  if (backend_state == NULL || publisher == NULL) {
    return;
  }

  current = &backend_state->publishers;
  while (*current != NULL) {
    if (*current == publisher) {
      *current = publisher->next;
      return;
    }
    current = &(*current)->next;
  }
}

static grcl_result_t grcl_inprocess_destroy_publisher(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_publisher_state_t * backend_publisher)
{
  if (backend_state == NULL || backend_publisher == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_inprocess_remove_publisher_link(backend_state, backend_publisher);
  free(backend_publisher->topic_name);
  free(backend_publisher);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_create_subscription(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node,
  const grcl_subscription_t * subscription,
  const grcl_subscription_options_t * options,
  grcl_backend_subscription_state_t ** out_backend_subscription)
{
  grcl_backend_subscription_state_t * created;

  (void)subscription;

  if (backend_state == NULL || backend_node == NULL || options == NULL ||
    options->topic_name == NULL || options->type_support == NULL ||
    out_backend_subscription == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_subscription = NULL;
  created = (grcl_backend_subscription_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->topic_name = grcl_inprocess_copy_string(options->topic_name);
  if (created->topic_name == NULL) {
    free(created);
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->runtime = backend_state;
  created->node = backend_node;
  created->type_id = options->type_support->type_id;
  created->next = backend_state->subscriptions;
  backend_state->subscriptions = created;
  *out_backend_subscription = created;
  return GRCL_OK;
}

static void grcl_inprocess_remove_subscription_link(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_subscription_state_t * subscription)
{
  grcl_backend_subscription_state_t ** current;

  if (backend_state == NULL || subscription == NULL) {
    return;
  }

  current = &backend_state->subscriptions;
  while (*current != NULL) {
    if (*current == subscription) {
      *current = subscription->next;
      return;
    }
    current = &(*current)->next;
  }
}

static grcl_result_t grcl_inprocess_destroy_subscription(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_subscription_state_t * backend_subscription)
{
  if (backend_state == NULL || backend_subscription == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_inprocess_remove_subscription_link(backend_state, backend_subscription);
  free(backend_subscription->topic_name);
  free(backend_subscription);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_publish_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_publisher_state_t * backend_publisher,
  const void * payload,
  size_t payload_size)
{
  if (backend_state == NULL || backend_publisher == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  return grcl_inprocess_queue_push(&backend_publisher->pending, payload, payload_size);
}

static grcl_result_t grcl_inprocess_subscription_take_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_subscription_state_t * backend_subscription,
  void * out_payload,
  size_t payload_capacity,
  size_t * out_payload_size)
{
  grcl_inprocess_message_t * message = NULL;
  grcl_result_t result;

  if (backend_state == NULL || backend_subscription == NULL ||
    out_payload_size == NULL || (payload_capacity > 0u && out_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  result = grcl_inprocess_queue_peek(&backend_subscription->ready, &message);
  if (result != GRCL_OK) {
    return result;
  }

  *out_payload_size = message->size;
  if (payload_capacity < message->size) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  if (message->size > 0u) {
    memcpy(out_payload, message->data, message->size);
  }
  grcl_inprocess_queue_pop(&backend_subscription->ready);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_create_service(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node,
  const grcl_service_t * service,
  const grcl_service_options_t * options,
  grcl_backend_service_state_t ** out_backend_service)
{
  (void)backend_state;
  (void)backend_node;
  (void)service;
  (void)options;
  (void)out_backend_service;
  return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
}

static grcl_result_t grcl_inprocess_create_client(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node,
  const grcl_client_t * client,
  const grcl_client_options_t * options,
  grcl_backend_client_state_t ** out_backend_client)
{
  (void)backend_state;
  (void)backend_node;
  (void)client;
  (void)options;
  (void)out_backend_client;
  return GRCL_ERROR_UNSUPPORTED_CAPABILITY;
}

static grcl_result_t grcl_inprocess_create_executor(
  grcl_backend_runtime_state_t * backend_state,
  const grcl_executor_t * executor,
  const grcl_executor_options_t * options,
  grcl_backend_executor_state_t ** out_backend_executor)
{
  grcl_backend_executor_state_t * created;

  (void)executor;
  (void)options;

  if (backend_state == NULL || out_backend_executor == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_executor = NULL;
  created = (grcl_backend_executor_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  *out_backend_executor = created;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_destroy_executor(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor)
{
  if (backend_state == NULL || backend_executor == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  free(backend_executor);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_executor_add_node(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor,
  grcl_backend_node_state_t * backend_node)
{
  if (backend_state == NULL || backend_executor == NULL || backend_node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  for (size_t i = 0u; i < backend_executor->node_count; ++i) {
    if (backend_executor->nodes[i] == backend_node) {
      return GRCL_OK;
    }
  }
  if (backend_executor->node_count >= GRCL_INPROCESS_QUEUE_CAPACITY) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  backend_executor->nodes[backend_executor->node_count++] = backend_node;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_executor_remove_node(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor,
  grcl_backend_node_state_t * backend_node)
{
  (void)backend_state;

  if (backend_executor == NULL || backend_node == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  for (size_t i = 0u; i < backend_executor->node_count; ++i) {
    if (backend_executor->nodes[i] == backend_node) {
      for (size_t j = i + 1u; j < backend_executor->node_count; ++j) {
        backend_executor->nodes[j - 1u] = backend_executor->nodes[j];
      }
      --backend_executor->node_count;
      backend_executor->nodes[backend_executor->node_count] = NULL;
      return GRCL_OK;
    }
  }

  return GRCL_ERROR_NOT_FOUND;
}

static int grcl_inprocess_executor_contains_node(
  const grcl_backend_executor_state_t * executor,
  const grcl_backend_node_state_t * node)
{
  if (executor == NULL || node == NULL) {
    return 0;
  }

  for (size_t i = 0u; i < executor->node_count; ++i) {
    if (executor->nodes[i] == node) {
      return 1;
    }
  }

  return 0;
}

static grcl_result_t grcl_inprocess_dispatch_one_message(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor,
  grcl_backend_publisher_state_t * publisher)
{
  grcl_inprocess_message_t * message = NULL;
  grcl_backend_subscription_state_t * subscription;
  grcl_result_t result;

  if (!grcl_inprocess_executor_contains_node(backend_executor, publisher->node)) {
    return GRCL_OK;
  }

  result = grcl_inprocess_queue_peek(&publisher->pending, &message);
  if (result != GRCL_OK) {
    return result;
  }

  for (subscription = backend_state->subscriptions;
    subscription != NULL; subscription = subscription->next) {
    if (grcl_inprocess_executor_contains_node(backend_executor, subscription->node) &&
      grcl_inprocess_endpoint_matches(publisher, subscription) &&
      !grcl_inprocess_queue_has_capacity(&subscription->ready)) {
      return GRCL_ERROR_CAPACITY_EXCEEDED;
    }
  }

  for (subscription = backend_state->subscriptions;
    subscription != NULL; subscription = subscription->next) {
    if (!grcl_inprocess_executor_contains_node(backend_executor, subscription->node) ||
      !grcl_inprocess_endpoint_matches(publisher, subscription)) {
      continue;
    }

    result = grcl_inprocess_queue_push(
      &subscription->ready,
      message->data,
      message->size);
    if (result != GRCL_OK) {
      return result;
    }
  }

  grcl_inprocess_queue_pop(&publisher->pending);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_executor_spin_once(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor,
  uint64_t timeout_ns)
{
  grcl_backend_publisher_state_t * publisher;

  (void)timeout_ns;

  if (backend_state == NULL || backend_executor == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  for (publisher = backend_state->publishers;
    publisher != NULL; publisher = publisher->next) {
    if (!grcl_inprocess_executor_contains_node(backend_executor, publisher->node)) {
      continue;
    }

    while (publisher->pending.count > 0u) {
      grcl_result_t result = grcl_inprocess_dispatch_one_message(
        backend_state,
        backend_executor,
        publisher);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }

  return GRCL_OK;
}

static const grcl_backend_ops_t grcl_native_inprocess_backend_ops = {
  .struct_size = sizeof(grcl_backend_ops_t),
  .abi_version = GRCL_C_ABI_VERSION_CURRENT,
  .create_runtime = grcl_inprocess_create_runtime,
  .start_runtime = grcl_inprocess_start_runtime,
  .stop_runtime = grcl_inprocess_stop_runtime,
  .destroy_runtime = grcl_inprocess_destroy_runtime,
  .get_capabilities = grcl_inprocess_get_capabilities,
  .negotiate_capabilities = grcl_inprocess_negotiate_capabilities,
  .get_diagnostics = grcl_inprocess_get_diagnostics,
  .create_node = grcl_inprocess_create_node,
  .destroy_node = grcl_inprocess_destroy_node,
  .create_publisher = grcl_inprocess_create_publisher,
  .destroy_publisher = grcl_inprocess_destroy_publisher,
  .create_subscription = grcl_inprocess_create_subscription,
  .destroy_subscription = grcl_inprocess_destroy_subscription,
  .publish_bytes = grcl_inprocess_publish_bytes,
  .subscription_take_bytes = grcl_inprocess_subscription_take_bytes,
  .create_service = grcl_inprocess_create_service,
  .create_client = grcl_inprocess_create_client,
  .create_executor = grcl_inprocess_create_executor,
  .destroy_executor = grcl_inprocess_destroy_executor,
  .executor_add_node = grcl_inprocess_executor_add_node,
  .executor_remove_node = grcl_inprocess_executor_remove_node,
  .executor_spin_once = grcl_inprocess_executor_spin_once
};

static const grcl_backend_descriptor_t grcl_native_inprocess_backend_descriptor_record = {
  sizeof(grcl_backend_descriptor_t),
  GRCL_C_ABI_VERSION_CURRENT,
  "native/inprocess",
  GRCL_BACKEND_FAMILY_ID_NATIVE,
  0u,
  1u,
  0u,
  &grcl_native_inprocess_backend_ops
};

const grcl_backend_descriptor_t * grcl_native_inprocess_backend_descriptor(void)
{
  return &grcl_native_inprocess_backend_descriptor_record;
}
