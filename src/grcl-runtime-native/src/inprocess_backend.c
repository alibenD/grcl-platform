#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <grcl/c/backend.h>
#include <grcl/c/version.h>

#define GRCL_INPROCESS_MAX_PAYLOAD_BYTES 1024u
#define GRCL_INPROCESS_QUEUE_CAPACITY 8u
#define GRCL_INPROCESS_MAX_PARAMETERS 8u
#define GRCL_INPROCESS_PARAMETER_NAME_BYTES 128u
#define GRCL_INPROCESS_PARAMETER_VALUE_BYTES 1024u

typedef struct grcl_inprocess_message {
  size_t size;
  unsigned char data[GRCL_INPROCESS_MAX_PAYLOAD_BYTES];
} grcl_inprocess_message_t;

typedef struct grcl_inprocess_message_queue {
  grcl_inprocess_message_t messages[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t head;
  size_t count;
} grcl_inprocess_message_queue_t;

typedef enum grcl_inprocess_request_state {
  GRCL_INPROCESS_REQUEST_STATE_UNUSED = 0,
  GRCL_INPROCESS_REQUEST_STATE_PENDING_REQUEST = 1,
  GRCL_INPROCESS_REQUEST_STATE_READY_REQUEST = 2,
  GRCL_INPROCESS_REQUEST_STATE_TAKEN_REQUEST = 3,
  GRCL_INPROCESS_REQUEST_STATE_PENDING_RESPONSE = 4,
  GRCL_INPROCESS_REQUEST_STATE_READY_RESPONSE = 5
} grcl_inprocess_request_state_t;

typedef struct grcl_inprocess_request_record {
  grcl_request_id_t id;
  struct grcl_backend_client_state * client;
  struct grcl_backend_service_state * service;
  grcl_inprocess_request_state_t state;
} grcl_inprocess_request_record_t;

typedef struct grcl_inprocess_request_message {
  grcl_request_id_t id;
  struct grcl_backend_client_state * client;
  struct grcl_backend_service_state * service;
  size_t size;
  unsigned char data[GRCL_INPROCESS_MAX_PAYLOAD_BYTES];
} grcl_inprocess_request_message_t;

typedef struct grcl_inprocess_request_queue {
  grcl_inprocess_request_message_t messages[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t head;
  size_t count;
} grcl_inprocess_request_queue_t;

typedef struct grcl_inprocess_response_message {
  grcl_request_id_t id;
  struct grcl_backend_client_state * client;
  struct grcl_backend_service_state * service;
  size_t size;
  unsigned char data[GRCL_INPROCESS_MAX_PAYLOAD_BYTES];
} grcl_inprocess_response_message_t;

typedef struct grcl_inprocess_response_queue {
  grcl_inprocess_response_message_t messages[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t head;
  size_t count;
} grcl_inprocess_response_queue_t;

typedef struct grcl_inprocess_param_entry {
  int in_use;
  size_t name_offset;
  size_t name_size;
  grcl_param_type_t type;
  size_t value_offset;
  size_t value_size;
} grcl_inprocess_param_entry_t;

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
  struct grcl_backend_runtime_state * runtime;
  grcl_backend_node_state_t * node;
  char * service_name;
  uint64_t request_type_id;
  uint64_t response_type_id;
  grcl_inprocess_request_queue_t ready_requests;
  struct grcl_backend_service_state * next;
};

struct grcl_backend_client_state {
  struct grcl_backend_runtime_state * runtime;
  grcl_backend_node_state_t * node;
  char * service_name;
  uint64_t request_type_id;
  uint64_t response_type_id;
  grcl_inprocess_response_queue_t ready_responses;
  struct grcl_backend_client_state * next;
};

struct grcl_backend_executor_state {
  grcl_backend_node_state_t * nodes[GRCL_INPROCESS_QUEUE_CAPACITY];
  size_t node_count;
};

struct grcl_backend_runtime_state {
  int started;
  grcl_request_id_t next_request_id;
  grcl_backend_node_state_t * nodes;
  grcl_backend_publisher_state_t * publishers;
  grcl_backend_subscription_state_t * subscriptions;
  grcl_backend_service_state_t * services;
  grcl_backend_client_state_t * clients;
  grcl_inprocess_request_queue_t pending_requests;
  grcl_inprocess_response_queue_t pending_responses;
  grcl_inprocess_request_record_t request_records[GRCL_INPROCESS_QUEUE_CAPACITY * 4u];
  grcl_inprocess_param_entry_t params[GRCL_INPROCESS_MAX_PARAMETERS];
  size_t param_count;
  size_t param_name_bytes_used;
  size_t param_value_bytes_used;
  unsigned char param_name_storage[GRCL_INPROCESS_PARAMETER_NAME_BYTES];
  unsigned char param_value_storage[GRCL_INPROCESS_PARAMETER_VALUE_BYTES];
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

static int grcl_inprocess_name_valid(const char * value)
{
  return value != NULL && value[0] != '\0';
}

static int grcl_inprocess_param_type_valid(grcl_param_type_t type)
{
  return type == GRCL_PARAM_TYPE_BOOL ||
    type == GRCL_PARAM_TYPE_INT64 ||
    type == GRCL_PARAM_TYPE_UINT64 ||
    type == GRCL_PARAM_TYPE_FLOAT64 ||
    type == GRCL_PARAM_TYPE_STRING ||
    type == GRCL_PARAM_TYPE_BYTES;
}

static const char * grcl_inprocess_param_name(
  const grcl_backend_runtime_state_t * backend_state,
  const grcl_inprocess_param_entry_t * entry)
{
  if (backend_state == NULL || entry == NULL || !entry->in_use) {
    return NULL;
  }

  return (const char *)(backend_state->param_name_storage + entry->name_offset);
}

static const unsigned char * grcl_inprocess_param_value(
  const grcl_backend_runtime_state_t * backend_state,
  const grcl_inprocess_param_entry_t * entry)
{
  if (backend_state == NULL || entry == NULL || !entry->in_use) {
    return NULL;
  }

  return backend_state->param_value_storage + entry->value_offset;
}

static grcl_inprocess_param_entry_t * grcl_inprocess_find_param_entry(
  grcl_backend_runtime_state_t * backend_state,
  const char * name)
{
  if (backend_state == NULL || !grcl_inprocess_name_valid(name)) {
    return NULL;
  }

  for (size_t i = 0u; i < GRCL_INPROCESS_MAX_PARAMETERS; ++i) {
    grcl_inprocess_param_entry_t * entry = &backend_state->params[i];
    const char * entry_name = NULL;

    if (!entry->in_use) {
      continue;
    }

    entry_name = grcl_inprocess_param_name(backend_state, entry);
    if (entry_name != NULL && strcmp(entry_name, name) == 0) {
      return entry;
    }
  }

  return NULL;
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

static grcl_result_t grcl_inprocess_request_queue_push(
  grcl_inprocess_request_queue_t * queue,
  grcl_request_id_t request_id,
  grcl_backend_client_state_t * client,
  grcl_backend_service_state_t * service,
  const void * payload,
  size_t payload_size)
{
  size_t index;

  if (queue == NULL || client == NULL || service == NULL ||
    (payload_size > 0u && payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (payload_size > GRCL_INPROCESS_MAX_PAYLOAD_BYTES) {
    return GRCL_ERROR_PAYLOAD_TOO_LARGE;
  }
  if (queue->count >= GRCL_INPROCESS_QUEUE_CAPACITY) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  index = (queue->head + queue->count) % GRCL_INPROCESS_QUEUE_CAPACITY;
  queue->messages[index].id = request_id;
  queue->messages[index].client = client;
  queue->messages[index].service = service;
  queue->messages[index].size = payload_size;
  if (payload_size > 0u) {
    memcpy(queue->messages[index].data, payload, payload_size);
  }
  ++queue->count;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_request_queue_peek(
  grcl_inprocess_request_queue_t * queue,
  grcl_inprocess_request_message_t ** out_message)
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

static void grcl_inprocess_request_queue_pop(grcl_inprocess_request_queue_t * queue)
{
  if (queue == NULL || queue->count == 0u) {
    return;
  }

  queue->head = (queue->head + 1u) % GRCL_INPROCESS_QUEUE_CAPACITY;
  --queue->count;
}

static int grcl_inprocess_request_queue_has_capacity(
  const grcl_inprocess_request_queue_t * queue)
{
  return queue != NULL && queue->count < GRCL_INPROCESS_QUEUE_CAPACITY;
}

static grcl_result_t grcl_inprocess_response_queue_push(
  grcl_inprocess_response_queue_t * queue,
  grcl_request_id_t request_id,
  grcl_backend_client_state_t * client,
  grcl_backend_service_state_t * service,
  const void * payload,
  size_t payload_size)
{
  size_t index;

  if (queue == NULL || client == NULL || service == NULL ||
    (payload_size > 0u && payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (payload_size > GRCL_INPROCESS_MAX_PAYLOAD_BYTES) {
    return GRCL_ERROR_PAYLOAD_TOO_LARGE;
  }
  if (queue->count >= GRCL_INPROCESS_QUEUE_CAPACITY) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  index = (queue->head + queue->count) % GRCL_INPROCESS_QUEUE_CAPACITY;
  queue->messages[index].id = request_id;
  queue->messages[index].client = client;
  queue->messages[index].service = service;
  queue->messages[index].size = payload_size;
  if (payload_size > 0u) {
    memcpy(queue->messages[index].data, payload, payload_size);
  }
  ++queue->count;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_response_queue_peek(
  grcl_inprocess_response_queue_t * queue,
  grcl_inprocess_response_message_t ** out_message)
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

static void grcl_inprocess_response_queue_pop(grcl_inprocess_response_queue_t * queue)
{
  if (queue == NULL || queue->count == 0u) {
    return;
  }

  queue->head = (queue->head + 1u) % GRCL_INPROCESS_QUEUE_CAPACITY;
  --queue->count;
}

static int grcl_inprocess_response_queue_has_capacity(
  const grcl_inprocess_response_queue_t * queue)
{
  return queue != NULL && queue->count < GRCL_INPROCESS_QUEUE_CAPACITY;
}

static grcl_inprocess_request_record_t * grcl_inprocess_find_request_record(
  grcl_backend_runtime_state_t * backend_state,
  grcl_request_id_t request_id)
{
  if (backend_state == NULL || request_id == 0u) {
    return NULL;
  }

  for (size_t i = 0u; i < GRCL_INPROCESS_QUEUE_CAPACITY * 4u; ++i) {
    if (backend_state->request_records[i].state != GRCL_INPROCESS_REQUEST_STATE_UNUSED &&
      backend_state->request_records[i].id == request_id) {
      return &backend_state->request_records[i];
    }
  }

  return NULL;
}

static grcl_inprocess_request_record_t * grcl_inprocess_allocate_request_record(
  grcl_backend_runtime_state_t * backend_state,
  grcl_request_id_t request_id,
  grcl_backend_client_state_t * client,
  grcl_backend_service_state_t * service)
{
  if (backend_state == NULL || request_id == 0u || client == NULL || service == NULL) {
    return NULL;
  }

  for (size_t i = 0u; i < GRCL_INPROCESS_QUEUE_CAPACITY * 4u; ++i) {
    if (backend_state->request_records[i].state == GRCL_INPROCESS_REQUEST_STATE_UNUSED) {
      backend_state->request_records[i].id = request_id;
      backend_state->request_records[i].client = client;
      backend_state->request_records[i].service = service;
      backend_state->request_records[i].state =
        GRCL_INPROCESS_REQUEST_STATE_PENDING_REQUEST;
      return &backend_state->request_records[i];
    }
  }

  return NULL;
}

static void grcl_inprocess_clear_request_record(
  grcl_inprocess_request_record_t * record)
{
  if (record == NULL) {
    return;
  }

  memset(record, 0, sizeof(*record));
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

static int grcl_inprocess_service_name_matches(
  const grcl_backend_service_state_t * service,
  const char * service_name)
{
  return service != NULL && service->service_name != NULL && service_name != NULL &&
    strcmp(service->service_name, service_name) == 0;
}

static int grcl_inprocess_service_client_matches(
  const grcl_backend_service_state_t * service,
  const grcl_backend_client_state_t * client)
{
  return service != NULL && client != NULL &&
    service->request_type_id == client->request_type_id &&
    service->response_type_id == client->response_type_id &&
    grcl_inprocess_service_name_matches(service, client->service_name);
}

static grcl_backend_service_state_t * grcl_inprocess_find_matching_service(
  grcl_backend_runtime_state_t * backend_state,
  const grcl_backend_client_state_t * client,
  int * out_name_found)
{
  grcl_backend_service_state_t * service;

  if (out_name_found != NULL) {
    *out_name_found = 0;
  }
  if (backend_state == NULL || client == NULL) {
    return NULL;
  }

  for (service = backend_state->services; service != NULL; service = service->next) {
    if (!grcl_inprocess_service_name_matches(service, client->service_name)) {
      continue;
    }
    if (out_name_found != NULL) {
      *out_name_found = 1;
    }
    if (grcl_inprocess_service_client_matches(service, client)) {
      return service;
    }
  }

  return NULL;
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

  state->next_request_id = 1u;
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
    GRCL_CAPABILITY_SUMMARY_FLAG_POLL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_SEQUENTIAL_EXECUTOR |
    GRCL_CAPABILITY_SUMMARY_FLAG_BASIC_DIAGNOSTICS |
    GRCL_CAPABILITY_SUMMARY_FLAG_RUNTIME_LOCAL_PARAMS;
  out_record->max_payload_bytes = GRCL_INPROCESS_MAX_PAYLOAD_BYTES;
  out_record->max_entities = 32u;
  out_record->max_nodes = 8u;
  out_record->max_endpoints = 32u;
  out_record->max_publishers = 16u;
  out_record->max_subscriptions = 16u;
  out_record->max_services = 16u;
  out_record->max_clients = 16u;
  out_record->message_buffer_bytes =
    GRCL_INPROCESS_MAX_PAYLOAD_BYTES * GRCL_INPROCESS_QUEUE_CAPACITY;
  out_record->executor_state_bytes = sizeof(grcl_backend_executor_state_t);
  out_record->max_parameters = GRCL_INPROCESS_MAX_PARAMETERS;
  out_record->parameter_name_buffer_bytes =
    GRCL_INPROCESS_PARAMETER_NAME_BYTES;
  out_record->parameter_value_buffer_bytes =
    GRCL_INPROCESS_PARAMETER_VALUE_BYTES;
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

static grcl_result_t grcl_inprocess_runtime_param_set(
  grcl_backend_runtime_state_t * backend_state,
  const grcl_param_record_t * param)
{
  grcl_inprocess_param_entry_t * existing_entry;
  grcl_inprocess_param_entry_t new_entries[GRCL_INPROCESS_MAX_PARAMETERS];
  unsigned char new_name_storage[GRCL_INPROCESS_PARAMETER_NAME_BYTES];
  unsigned char new_value_storage[GRCL_INPROCESS_PARAMETER_VALUE_BYTES];
  size_t name_bytes_used = 0u;
  size_t value_bytes_used = 0u;
  size_t entry_count = 0u;
  size_t name_size;
  int replaced = 0;

  if (backend_state == NULL || param == NULL ||
    !grcl_inprocess_name_valid(param->name) ||
    !grcl_inprocess_param_type_valid(param->type) ||
    (param->value_size > 0u && param->value == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  existing_entry = grcl_inprocess_find_param_entry(backend_state, param->name);
  if (existing_entry == NULL &&
    backend_state->param_count >= GRCL_INPROCESS_MAX_PARAMETERS) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  memset(new_entries, 0, sizeof(new_entries));
  memset(new_name_storage, 0, sizeof(new_name_storage));
  memset(new_value_storage, 0, sizeof(new_value_storage));
  name_size = strlen(param->name) + 1u;

  for (size_t i = 0u; i < GRCL_INPROCESS_MAX_PARAMETERS; ++i) {
    const grcl_inprocess_param_entry_t * source = &backend_state->params[i];
    const char * source_name = NULL;
    const unsigned char * source_value = NULL;
    const char * write_name = NULL;
    const void * write_value = NULL;
    size_t write_name_size = 0u;
    size_t write_value_size = 0u;
    grcl_param_type_t write_type = GRCL_PARAM_TYPE_UNKNOWN;

    if (!source->in_use) {
      continue;
    }

    if (source == existing_entry) {
      write_name = param->name;
      write_name_size = name_size;
      write_value = param->value;
      write_value_size = param->value_size;
      write_type = param->type;
      replaced = 1;
    } else {
      source_name = grcl_inprocess_param_name(backend_state, source);
      source_value = grcl_inprocess_param_value(backend_state, source);
      write_name = source_name;
      write_name_size = source->name_size;
      write_value = source_value;
      write_value_size = source->value_size;
      write_type = source->type;
    }

    if (name_bytes_used + write_name_size > GRCL_INPROCESS_PARAMETER_NAME_BYTES ||
      value_bytes_used + write_value_size > GRCL_INPROCESS_PARAMETER_VALUE_BYTES) {
      return GRCL_ERROR_CAPACITY_EXCEEDED;
    }

    memcpy(new_name_storage + name_bytes_used, write_name, write_name_size);
    if (write_value_size > 0u) {
      memcpy(new_value_storage + value_bytes_used, write_value, write_value_size);
    }

    new_entries[entry_count].in_use = 1;
    new_entries[entry_count].name_offset = name_bytes_used;
    new_entries[entry_count].name_size = write_name_size;
    new_entries[entry_count].type = write_type;
    new_entries[entry_count].value_offset = value_bytes_used;
    new_entries[entry_count].value_size = write_value_size;

    name_bytes_used += write_name_size;
    value_bytes_used += write_value_size;
    ++entry_count;
  }

  if (!replaced) {
    if (name_bytes_used + name_size > GRCL_INPROCESS_PARAMETER_NAME_BYTES ||
      value_bytes_used + param->value_size > GRCL_INPROCESS_PARAMETER_VALUE_BYTES) {
      return GRCL_ERROR_CAPACITY_EXCEEDED;
    }

    memcpy(new_name_storage + name_bytes_used, param->name, name_size);
    if (param->value_size > 0u) {
      memcpy(new_value_storage + value_bytes_used, param->value, param->value_size);
    }

    new_entries[entry_count].in_use = 1;
    new_entries[entry_count].name_offset = name_bytes_used;
    new_entries[entry_count].name_size = name_size;
    new_entries[entry_count].type = param->type;
    new_entries[entry_count].value_offset = value_bytes_used;
    new_entries[entry_count].value_size = param->value_size;

    name_bytes_used += name_size;
    value_bytes_used += param->value_size;
    ++entry_count;
  }

  memset(backend_state->params, 0, sizeof(backend_state->params));
  memset(backend_state->param_name_storage, 0, sizeof(backend_state->param_name_storage));
  memset(backend_state->param_value_storage, 0, sizeof(backend_state->param_value_storage));
  memcpy(backend_state->params, new_entries, sizeof(new_entries));
  if (name_bytes_used > 0u) {
    memcpy(backend_state->param_name_storage, new_name_storage, name_bytes_used);
  }
  if (value_bytes_used > 0u) {
    memcpy(backend_state->param_value_storage, new_value_storage, value_bytes_used);
  }
  backend_state->param_count = entry_count;
  backend_state->param_name_bytes_used = name_bytes_used;
  backend_state->param_value_bytes_used = value_bytes_used;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_runtime_param_get(
  grcl_backend_runtime_state_t * backend_state,
  const char * name,
  grcl_param_record_t * out_param,
  void * value_buffer,
  size_t value_buffer_capacity,
  size_t * out_value_size)
{
  grcl_inprocess_param_entry_t * entry;
  const char * stored_name;
  const unsigned char * stored_value;

  if (backend_state == NULL || !grcl_inprocess_name_valid(name) ||
    out_param == NULL || out_value_size == NULL ||
    (value_buffer_capacity > 0u && value_buffer == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  memset(out_param, 0, sizeof(*out_param));
  entry = grcl_inprocess_find_param_entry(backend_state, name);
  if (entry == NULL) {
    return GRCL_ERROR_NOT_FOUND;
  }

  *out_value_size = entry->value_size;
  if (value_buffer_capacity < entry->value_size) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  stored_name = grcl_inprocess_param_name(backend_state, entry);
  stored_value = grcl_inprocess_param_value(backend_state, entry);
  out_param->struct_size = sizeof(*out_param);
  out_param->abi_version = GRCL_C_ABI_VERSION_CURRENT;
  out_param->name = stored_name;
  out_param->type = entry->type;
  out_param->value = value_buffer;
  out_param->value_size = entry->value_size;
  if (entry->value_size > 0u) {
    memcpy(value_buffer, stored_value, entry->value_size);
  }

  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_runtime_param_list(
  grcl_backend_runtime_state_t * backend_state,
  char * out_names,
  size_t names_capacity,
  size_t * out_names_size,
  size_t * out_param_count)
{
  size_t required_size = 0u;
  size_t written = 0u;

  if (backend_state == NULL || out_names_size == NULL || out_param_count == NULL ||
    (names_capacity > 0u && out_names == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_param_count = backend_state->param_count;
  for (size_t i = 0u; i < backend_state->param_count; ++i) {
    const grcl_inprocess_param_entry_t * entry = &backend_state->params[i];

    if (!entry->in_use) {
      continue;
    }

    required_size += entry->name_size - 1u;
    if (written + 1u < backend_state->param_count) {
      required_size += 1u;
    }
    ++written;
  }
  *out_names_size = required_size;

  if (names_capacity < required_size) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  written = 0u;
  for (size_t i = 0u; i < backend_state->param_count; ++i) {
    const grcl_inprocess_param_entry_t * entry = &backend_state->params[i];
    const char * stored_name;
    size_t copy_size;

    if (!entry->in_use) {
      continue;
    }

    stored_name = grcl_inprocess_param_name(backend_state, entry);
    copy_size = entry->name_size - 1u;
    if (copy_size > 0u) {
      memcpy(out_names + written, stored_name, copy_size);
      written += copy_size;
    }
    if (written < required_size) {
      out_names[written++] = '\n';
    }
  }

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
  grcl_backend_service_state_t * created;

  (void)service;

  if (backend_state == NULL || backend_node == NULL || options == NULL ||
    options->service_name == NULL || options->request_type_support == NULL ||
    options->response_type_support == NULL || out_backend_service == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_service = NULL;
  created = (grcl_backend_service_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->service_name = grcl_inprocess_copy_string(options->service_name);
  if (created->service_name == NULL) {
    free(created);
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->runtime = backend_state;
  created->node = backend_node;
  created->request_type_id = options->request_type_support->type_id;
  created->response_type_id = options->response_type_support->type_id;
  created->next = backend_state->services;
  backend_state->services = created;
  *out_backend_service = created;
  return GRCL_OK;
}

static void grcl_inprocess_remove_service_link(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_service_state_t * service)
{
  grcl_backend_service_state_t ** current;

  if (backend_state == NULL || service == NULL) {
    return;
  }

  current = &backend_state->services;
  while (*current != NULL) {
    if (*current == service) {
      *current = service->next;
      return;
    }
    current = &(*current)->next;
  }
}

static grcl_result_t grcl_inprocess_destroy_service(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_service_state_t * backend_service)
{
  if (backend_state == NULL || backend_service == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_inprocess_remove_service_link(backend_state, backend_service);
  free(backend_service->service_name);
  free(backend_service);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_create_client(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_node_state_t * backend_node,
  const grcl_client_t * client,
  const grcl_client_options_t * options,
  grcl_backend_client_state_t ** out_backend_client)
{
  grcl_backend_client_state_t * created;

  (void)client;

  if (backend_state == NULL || backend_node == NULL || options == NULL ||
    options->service_name == NULL || options->request_type_support == NULL ||
    options->response_type_support == NULL || out_backend_client == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  *out_backend_client = NULL;
  created = (grcl_backend_client_state_t *)calloc(1u, sizeof(*created));
  if (created == NULL) {
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->service_name = grcl_inprocess_copy_string(options->service_name);
  if (created->service_name == NULL) {
    free(created);
    return GRCL_ERROR_OUT_OF_MEMORY;
  }

  created->runtime = backend_state;
  created->node = backend_node;
  created->request_type_id = options->request_type_support->type_id;
  created->response_type_id = options->response_type_support->type_id;
  created->next = backend_state->clients;
  backend_state->clients = created;
  *out_backend_client = created;
  return GRCL_OK;
}

static void grcl_inprocess_remove_client_link(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_client_state_t * client)
{
  grcl_backend_client_state_t ** current;

  if (backend_state == NULL || client == NULL) {
    return;
  }

  current = &backend_state->clients;
  while (*current != NULL) {
    if (*current == client) {
      *current = client->next;
      return;
    }
    current = &(*current)->next;
  }
}

static grcl_result_t grcl_inprocess_destroy_client(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_client_state_t * backend_client)
{
  if (backend_state == NULL || backend_client == NULL) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }

  grcl_inprocess_remove_client_link(backend_state, backend_client);
  free(backend_client->service_name);
  free(backend_client);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_client_send_request_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_client_state_t * backend_client,
  const void * request_payload,
  size_t request_payload_size,
  grcl_request_id_t * out_request_id)
{
  grcl_backend_service_state_t * service;
  grcl_inprocess_request_record_t * record;
  grcl_request_id_t request_id;
  int name_found = 0;
  grcl_result_t result;

  if (backend_state == NULL || backend_client == NULL || out_request_id == NULL ||
    (request_payload_size > 0u && request_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (request_payload_size > GRCL_INPROCESS_MAX_PAYLOAD_BYTES) {
    return GRCL_ERROR_PAYLOAD_TOO_LARGE;
  }

  service = grcl_inprocess_find_matching_service(backend_state, backend_client, &name_found);
  if (service == NULL) {
    return name_found ? GRCL_ERROR_TYPE_MISMATCH : GRCL_ERROR_PEER_UNAVAILABLE;
  }
  if (!grcl_inprocess_request_queue_has_capacity(&backend_state->pending_requests)) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  request_id = backend_state->next_request_id++;
  if (request_id == 0u) {
    request_id = backend_state->next_request_id++;
  }

  record = grcl_inprocess_allocate_request_record(
    backend_state,
    request_id,
    backend_client,
    service);
  if (record == NULL) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  result = grcl_inprocess_request_queue_push(
    &backend_state->pending_requests,
    request_id,
    backend_client,
    service,
    request_payload,
    request_payload_size);
  if (result != GRCL_OK) {
    grcl_inprocess_clear_request_record(record);
    return result;
  }

  *out_request_id = request_id;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_service_take_request_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_service_state_t * backend_service,
  void * out_request_payload,
  size_t request_payload_capacity,
  size_t * out_request_payload_size,
  grcl_request_id_t * out_request_id)
{
  grcl_inprocess_request_message_t * message = NULL;
  grcl_inprocess_request_record_t * record;
  grcl_result_t result;

  if (backend_state == NULL || backend_service == NULL ||
    out_request_payload_size == NULL || out_request_id == NULL ||
    (request_payload_capacity > 0u && out_request_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  result = grcl_inprocess_request_queue_peek(&backend_service->ready_requests, &message);
  if (result != GRCL_OK) {
    return result;
  }

  *out_request_payload_size = message->size;
  if (request_payload_capacity < message->size) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  if (message->size > 0u) {
    memcpy(out_request_payload, message->data, message->size);
  }
  *out_request_id = message->id;
  record = grcl_inprocess_find_request_record(backend_state, message->id);
  if (record != NULL && record->service == backend_service) {
    record->state = GRCL_INPROCESS_REQUEST_STATE_TAKEN_REQUEST;
  }
  grcl_inprocess_request_queue_pop(&backend_service->ready_requests);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_service_send_response_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_service_state_t * backend_service,
  grcl_request_id_t request_id,
  const void * response_payload,
  size_t response_payload_size)
{
  grcl_inprocess_request_record_t * record;
  grcl_result_t result;

  if (backend_state == NULL || backend_service == NULL ||
    (response_payload_size > 0u && response_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }
  if (response_payload_size > GRCL_INPROCESS_MAX_PAYLOAD_BYTES) {
    return GRCL_ERROR_PAYLOAD_TOO_LARGE;
  }

  record = grcl_inprocess_find_request_record(backend_state, request_id);
  if (record == NULL || record->service != backend_service ||
    record->state != GRCL_INPROCESS_REQUEST_STATE_TAKEN_REQUEST) {
    return GRCL_ERROR_NOT_FOUND;
  }
  if (!grcl_inprocess_response_queue_has_capacity(&backend_state->pending_responses) ||
    !grcl_inprocess_response_queue_has_capacity(&record->client->ready_responses)) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  result = grcl_inprocess_response_queue_push(
    &backend_state->pending_responses,
    request_id,
    record->client,
    backend_service,
    response_payload,
    response_payload_size);
  if (result != GRCL_OK) {
    return result;
  }

  record->state = GRCL_INPROCESS_REQUEST_STATE_PENDING_RESPONSE;
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_client_take_response_bytes(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_client_state_t * backend_client,
  grcl_request_id_t request_id,
  void * out_response_payload,
  size_t response_payload_capacity,
  size_t * out_response_payload_size)
{
  grcl_inprocess_response_message_t * message;
  grcl_inprocess_request_record_t * record;

  if (backend_state == NULL || backend_client == NULL ||
    out_response_payload_size == NULL ||
    (response_payload_capacity > 0u && out_response_payload == NULL)) {
    return GRCL_ERROR_INVALID_ARGUMENT;
  }
  if (!backend_state->started) {
    return GRCL_ERROR_BAD_STATE;
  }

  record = grcl_inprocess_find_request_record(backend_state, request_id);
  if (record == NULL || record->client != backend_client) {
    return GRCL_ERROR_NOT_FOUND;
  }

  for (size_t i = 0u; i < backend_client->ready_responses.count; ++i) {
    size_t index = (backend_client->ready_responses.head + i) % GRCL_INPROCESS_QUEUE_CAPACITY;
    message = &backend_client->ready_responses.messages[index];
    if (message->id != request_id) {
      continue;
    }

    *out_response_payload_size = message->size;
    if (response_payload_capacity < message->size) {
      return GRCL_ERROR_CAPACITY_EXCEEDED;
    }

    if (message->size > 0u) {
      memcpy(out_response_payload, message->data, message->size);
    }
    if (i == 0u) {
      grcl_inprocess_response_queue_pop(&backend_client->ready_responses);
    } else {
      for (size_t j = i + 1u; j < backend_client->ready_responses.count; ++j) {
        size_t from = (backend_client->ready_responses.head + j) % GRCL_INPROCESS_QUEUE_CAPACITY;
        size_t to = (backend_client->ready_responses.head + j - 1u) % GRCL_INPROCESS_QUEUE_CAPACITY;
        backend_client->ready_responses.messages[to] =
          backend_client->ready_responses.messages[from];
      }
      --backend_client->ready_responses.count;
    }
    grcl_inprocess_clear_request_record(record);
    return GRCL_OK;
  }

  return GRCL_ERROR_NO_DATA;
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

static grcl_result_t grcl_inprocess_dispatch_one_request(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor)
{
  grcl_inprocess_request_message_t * message = NULL;
  grcl_inprocess_request_record_t * record;
  grcl_result_t result;

  result = grcl_inprocess_request_queue_peek(&backend_state->pending_requests, &message);
  if (result != GRCL_OK) {
    return result == GRCL_ERROR_NO_DATA ? GRCL_OK : result;
  }
  if (!grcl_inprocess_executor_contains_node(backend_executor, message->client->node) ||
    !grcl_inprocess_executor_contains_node(backend_executor, message->service->node)) {
    return GRCL_ERROR_NO_DATA;
  }
  if (!grcl_inprocess_request_queue_has_capacity(&message->service->ready_requests)) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  result = grcl_inprocess_request_queue_push(
    &message->service->ready_requests,
    message->id,
    message->client,
    message->service,
    message->data,
    message->size);
  if (result != GRCL_OK) {
    return result;
  }

  record = grcl_inprocess_find_request_record(backend_state, message->id);
  if (record != NULL) {
    record->state = GRCL_INPROCESS_REQUEST_STATE_READY_REQUEST;
  }
  grcl_inprocess_request_queue_pop(&backend_state->pending_requests);
  return GRCL_OK;
}

static grcl_result_t grcl_inprocess_dispatch_one_response(
  grcl_backend_runtime_state_t * backend_state,
  grcl_backend_executor_state_t * backend_executor)
{
  grcl_inprocess_response_message_t * message = NULL;
  grcl_inprocess_request_record_t * record;
  grcl_result_t result;

  result = grcl_inprocess_response_queue_peek(&backend_state->pending_responses, &message);
  if (result != GRCL_OK) {
    return result == GRCL_ERROR_NO_DATA ? GRCL_OK : result;
  }
  if (!grcl_inprocess_executor_contains_node(backend_executor, message->service->node) ||
    !grcl_inprocess_executor_contains_node(backend_executor, message->client->node)) {
    return GRCL_ERROR_NO_DATA;
  }
  if (!grcl_inprocess_response_queue_has_capacity(&message->client->ready_responses)) {
    return GRCL_ERROR_CAPACITY_EXCEEDED;
  }

  result = grcl_inprocess_response_queue_push(
    &message->client->ready_responses,
    message->id,
    message->client,
    message->service,
    message->data,
    message->size);
  if (result != GRCL_OK) {
    return result;
  }

  record = grcl_inprocess_find_request_record(backend_state, message->id);
  if (record != NULL) {
    record->state = GRCL_INPROCESS_REQUEST_STATE_READY_RESPONSE;
  }
  grcl_inprocess_response_queue_pop(&backend_state->pending_responses);
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

    if (publisher->pending.count > 0u) {
      grcl_result_t result = grcl_inprocess_dispatch_one_message(
        backend_state,
        backend_executor,
        publisher);
      if (result != GRCL_OK) {
        return result;
      }
    }
  }

  if (backend_state->pending_requests.count > 0u) {
    grcl_result_t result = grcl_inprocess_dispatch_one_request(
      backend_state,
      backend_executor);
    if (result == GRCL_ERROR_NO_DATA) {
      return GRCL_OK;
    }
    if (result != GRCL_OK) {
      return result;
    }
  }

  if (backend_state->pending_responses.count > 0u) {
    grcl_result_t result = grcl_inprocess_dispatch_one_response(
      backend_state,
      backend_executor);
    if (result == GRCL_ERROR_NO_DATA) {
      return GRCL_OK;
    }
    if (result != GRCL_OK) {
      return result;
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
  .destroy_service = grcl_inprocess_destroy_service,
  .create_client = grcl_inprocess_create_client,
  .destroy_client = grcl_inprocess_destroy_client,
  .client_send_request_bytes = grcl_inprocess_client_send_request_bytes,
  .service_take_request_bytes = grcl_inprocess_service_take_request_bytes,
  .service_send_response_bytes = grcl_inprocess_service_send_response_bytes,
  .client_take_response_bytes = grcl_inprocess_client_take_response_bytes,
  .create_executor = grcl_inprocess_create_executor,
  .destroy_executor = grcl_inprocess_destroy_executor,
  .executor_add_node = grcl_inprocess_executor_add_node,
  .executor_remove_node = grcl_inprocess_executor_remove_node,
  .executor_spin_once = grcl_inprocess_executor_spin_once,
  .runtime_param_set = grcl_inprocess_runtime_param_set,
  .runtime_param_get = grcl_inprocess_runtime_param_get,
  .runtime_param_list = grcl_inprocess_runtime_param_list
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
