#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

static int expect_result(const char * label, grcl_result_t actual, grcl_result_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: result %d expected %d\n", label, actual, expected);
  return 1;
}

static int expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: size %zu expected %zu\n", label, actual, expected);
  return 1;
}

static int expect_request_id(const char * label, grcl_request_id_t actual, grcl_request_id_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(
    stderr,
    "FAIL %s: request id %llu expected %llu\n",
    label,
    (unsigned long long)actual,
    (unsigned long long)expected);
  return 1;
}

static int expect_type(
  const char * label,
  grcl_param_type_t actual,
  grcl_param_type_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: type %u expected %u\n", label, (unsigned)actual, (unsigned)expected);
  return 1;
}

static int expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (memcmp(actual, expected, size) == 0) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: bytes mismatch\n", label);
  return 1;
}

static int contains_name(const char * names, size_t names_size, const char * expected_name)
{
  size_t expected_size = strlen(expected_name);
  size_t start = 0u;

  while (start < names_size) {
    size_t end = start;
    while (end < names_size && names[end] != '\n') {
      ++end;
    }
    if ((end - start) == expected_size &&
      memcmp(names + start, expected_name, expected_size) == 0) {
      return 1;
    }
    start = end + 1u;
  }

  return 0;
}

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9010u,
  "example/core-message",
  0u,
  1u
};

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9011u,
  "example/core-request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9012u,
  "example/core-response",
  0u,
  1u
};

static grcl_runtime_options_t runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "core-middleware-example";
  options.profile_name = "native-inprocess";
  return options;
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/examples";
  return options;
}

static grcl_publisher_options_t publisher_options(void)
{
  grcl_publisher_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "examples/core.topic";
  options.type_support = &k_message_type;
  return options;
}

static grcl_subscription_options_t subscription_options(void)
{
  grcl_subscription_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "examples/core.topic";
  options.type_support = &k_message_type;
  return options;
}

static grcl_service_options_t service_options(void)
{
  grcl_service_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "examples/core.service";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_client_options_t client_options(void)
{
  grcl_client_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "examples/core.service";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_executor_options_t executor_options(void)
{
  grcl_executor_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static grcl_param_record_t param_record(
  const char * name,
  grcl_param_type_t type,
  const void * value,
  size_t value_size)
{
  grcl_param_record_t record = {0};
  record.struct_size = sizeof(record);
  record.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  record.name = name;
  record.type = type;
  record.value = value;
  record.value_size = value_size;
  return record;
}

int main(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * server_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t server_node_options = node_options("server");
  grcl_node_options_t client_node_options = node_options("client");
  grcl_publisher_options_t publisher_create_options = publisher_options();
  grcl_subscription_options_t subscription_create_options = subscription_options();
  grcl_service_options_t service_create_options = service_options();
  grcl_client_options_t client_create_options = client_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const uint8_t enabled = 1u;
  grcl_param_record_t enabled_record = param_record(
    "core.enabled",
    GRCL_PARAM_TYPE_BOOL,
    &enabled,
    sizeof(enabled));
  const unsigned char pub_payload[] = {0x01u, 0x02u, 0x03u, 0x04u};
  const unsigned char request[] = {0x90u, 0x91u};
  const unsigned char response[] = {0xa0u, 0xa1u, 0xa2u};
  unsigned char buffer[64] = {0};
  char names_buffer[64] = {0};
  size_t payload_size = 0u;
  size_t names_size = 0u;
  size_t param_count = 0u;
  grcl_param_record_t out_param = {0};
  grcl_request_id_t sent_id = 0u;
  grcl_request_id_t taken_id = 0u;
  int started = 0;
  int failures = 0;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(&runtime_create_options, &runtime),
      GRCL_OK) != 0 ||
    expect_result(
      "set runtime param",
      grcl_runtime_param_set(runtime, &enabled_record),
      GRCL_OK) != 0 ||
    expect_result(
      "create server node",
      grcl_node_create(runtime, &server_node_options, &server_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create client node",
      grcl_node_create(runtime, &client_node_options, &client_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create publisher",
      grcl_publisher_create(server_node, &publisher_create_options, &publisher),
      GRCL_OK) != 0 ||
    expect_result(
      "create subscription",
      grcl_subscription_create(client_node, &subscription_create_options, &subscription),
      GRCL_OK) != 0 ||
    expect_result(
      "create service",
      grcl_service_create(server_node, &service_create_options, &service),
      GRCL_OK) != 0 ||
    expect_result(
      "create client",
      grcl_client_create(client_node, &client_create_options, &client),
      GRCL_OK) != 0 ||
    expect_result(
      "create executor",
      grcl_executor_create(runtime, &executor_create_options, &executor),
      GRCL_OK) != 0 ||
    expect_result("add server node", grcl_executor_add_node(executor, server_node), GRCL_OK) != 0 ||
    expect_result("add client node", grcl_executor_add_node(executor, client_node), GRCL_OK) != 0 ||
    expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }
  started = 1;

  if (expect_result(
      "get runtime param",
      grcl_runtime_param_get(
        runtime,
        "core.enabled",
        &out_param,
        buffer,
        sizeof(buffer),
        &payload_size),
      GRCL_OK) != 0 ||
    expect_type("param type", out_param.type, GRCL_PARAM_TYPE_BOOL) != 0 ||
    expect_size("param size", payload_size, sizeof(enabled)) != 0 ||
    expect_bytes("param bytes", buffer, &enabled, sizeof(enabled)) != 0 ||
    expect_result(
      "publish payload",
      grcl_publisher_publish_bytes(publisher, pub_payload, sizeof(pub_payload)),
      GRCL_OK) != 0 ||
    expect_result("dispatch pub/sub", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take payload",
      grcl_subscription_take_bytes(subscription, buffer, sizeof(buffer), &payload_size),
      GRCL_OK) != 0 ||
    expect_size("pub/sub payload size", payload_size, sizeof(pub_payload)) != 0 ||
    expect_bytes("pub/sub payload", buffer, pub_payload, sizeof(pub_payload)) != 0 ||
    expect_result(
      "send request",
      grcl_client_send_request_bytes(client, request, sizeof(request), &sent_id),
      GRCL_OK) != 0 ||
    expect_result("dispatch request", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take request",
      grcl_service_take_request_bytes(service, buffer, sizeof(buffer), &payload_size, &taken_id),
      GRCL_OK) != 0 ||
    expect_request_id("request id", taken_id, sent_id) != 0 ||
    expect_size("request size", payload_size, sizeof(request)) != 0 ||
    expect_bytes("request bytes", buffer, request, sizeof(request)) != 0 ||
    expect_result(
      "send response",
      grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
      GRCL_OK) != 0 ||
    expect_result("dispatch response", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take response",
      grcl_client_take_response_bytes(client, sent_id, buffer, sizeof(buffer), &payload_size),
      GRCL_OK) != 0 ||
    expect_size("response size", payload_size, sizeof(response)) != 0 ||
    expect_bytes("response bytes", buffer, response, sizeof(response)) != 0 ||
    expect_result(
      "list params",
      grcl_runtime_param_list(
        runtime,
        names_buffer,
        sizeof(names_buffer),
        &names_size,
        &param_count),
      GRCL_OK) != 0 ||
    expect_size("param count", param_count, 1u) != 0) {
    failures = 1;
    goto cleanup;
  }

  if (!contains_name(names_buffer, names_size, "core.enabled")) {
    (void)fprintf(stderr, "FAIL param list missing core.enabled\n");
    failures = 1;
    goto cleanup;
  }

cleanup:
  if (runtime != NULL && started) {
    if (expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
      failures = 1;
    }
  }
  if (runtime != NULL) {
    if (expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK) != 0) {
      failures = 1;
    }
  }

  return failures;
}
