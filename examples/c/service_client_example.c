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

static int expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (memcmp(actual, expected, size) == 0) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: payload mismatch\n", label);
  return 1;
}

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9002u,
  "example/request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9003u,
  "example/response",
  0u,
  1u
};

static grcl_runtime_options_t runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "service-client-example";
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

static grcl_service_options_t service_options(void)
{
  grcl_service_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "examples/service.alpha";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_client_options_t client_options(void)
{
  grcl_client_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "examples/service.alpha";
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

enum
{
  k_max_delivery_spins = 8
};

int main(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t service_node_options = node_options("server");
  grcl_node_options_t client_node_options = node_options("client");
  grcl_service_options_t service_create_options = service_options();
  grcl_client_options_t client_create_options = client_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char request[] = {0x11u, 0x22u, 0x33u};
  const unsigned char response[] = {0xa1u, 0xb2u, 0xc3u, 0xd4u};
  unsigned char received[32] = {0};
  size_t received_size = 0u;
  grcl_request_id_t sent_id = 0u;
  grcl_request_id_t taken_id = 0u;
  int started = 0;
  int failures = 0;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(&runtime_create_options, &runtime),
      GRCL_OK) != 0 ||
    expect_result(
      "create service node",
      grcl_node_create(runtime, &service_node_options, &service_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create client node",
      grcl_node_create(runtime, &client_node_options, &client_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create service",
      grcl_service_create(service_node, &service_create_options, &service),
      GRCL_OK) != 0 ||
    expect_result(
      "create client",
      grcl_client_create(client_node, &client_create_options, &client),
      GRCL_OK) != 0 ||
    expect_result(
      "create executor",
      grcl_executor_create(runtime, &executor_create_options, &executor),
      GRCL_OK) != 0 ||
    expect_result("add service node", grcl_executor_add_node(executor, service_node), GRCL_OK) != 0 ||
    expect_result("add client node", grcl_executor_add_node(executor, client_node), GRCL_OK) != 0 ||
    expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }
  started = 1;

  if (expect_result(
      "send request",
      grcl_client_send_request_bytes(client, request, sizeof(request), &sent_id),
      GRCL_OK) != 0 ||
    expect_result(
      "response not ready before spin",
      grcl_client_take_response_bytes(
        client,
        sent_id,
        received,
        sizeof(received),
        &received_size),
      GRCL_ERROR_NO_DATA) != 0) {
    failures = 1;
    goto cleanup;
  }

  for (size_t attempt = 0u; attempt < k_max_delivery_spins; ++attempt) {
    grcl_result_t take_result = grcl_service_take_request_bytes(
      service,
      received,
      sizeof(received),
      &received_size,
      &taken_id);
    if (take_result == GRCL_OK) {
      break;
    }
    if (expect_result("request pending", take_result, GRCL_ERROR_NO_DATA) != 0 ||
      expect_result("dispatch request", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0) {
      failures = 1;
      goto cleanup;
    }

    if (attempt + 1u == k_max_delivery_spins) {
      (void)fprintf(
        stderr,
        "FAIL request delivery: exhausted %u spins without data\n",
        (unsigned)k_max_delivery_spins);
      failures = 1;
      goto cleanup;
    }
  }

  if (expect_request_id("request id preserved", taken_id, sent_id) != 0 ||
    expect_size("request size", received_size, sizeof(request)) != 0 ||
    expect_bytes("request payload", received, request, sizeof(request)) != 0 ||
    expect_result(
      "send response",
      grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
      GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }

  memset(received, 0, sizeof(received));

  for (size_t attempt = 0u; attempt < k_max_delivery_spins; ++attempt) {
    grcl_result_t take_result = grcl_client_take_response_bytes(
      client,
      sent_id,
      received,
      sizeof(received),
      &received_size);
    if (take_result == GRCL_OK) {
      break;
    }
    if (expect_result("response pending", take_result, GRCL_ERROR_NO_DATA) != 0 ||
      expect_result("dispatch response", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0) {
      failures = 1;
      goto cleanup;
    }

    if (attempt + 1u == k_max_delivery_spins) {
      (void)fprintf(
        stderr,
        "FAIL response delivery: exhausted %u spins without data\n",
        (unsigned)k_max_delivery_spins);
      failures = 1;
      goto cleanup;
    }
  }

  if (expect_size("response size", received_size, sizeof(response)) != 0 ||
    expect_bytes("response payload", received, response, sizeof(response)) != 0 ||
    expect_result(
      "request id no longer pending",
      grcl_client_take_response_bytes(
        client,
        sent_id,
        received,
        sizeof(received),
        &received_size),
      GRCL_ERROR_NOT_FOUND) != 0) {
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
