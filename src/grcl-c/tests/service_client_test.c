#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

static int expect_result(
  const char * label,
  grcl_result_t actual,
  grcl_result_t expected)
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

static int expect_nonzero_request_id(const char * label, grcl_request_id_t request_id)
{
  if (request_id != 0u) {
    return 0;
  }

  (void)fprintf(stderr, "FAIL %s: request id was zero\n", label);
  return 1;
}

static int expect_same_request_id(
  const char * label,
  grcl_request_id_t actual,
  grcl_request_id_t expected)
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

static int expect_request_id_greater(
  const char * label,
  grcl_request_id_t actual,
  grcl_request_id_t previous)
{
  if (actual > previous) {
    return 0;
  }

  (void)fprintf(
    stderr,
    "FAIL %s: request id %llu was not greater than %llu\n",
    label,
    (unsigned long long)actual,
    (unsigned long long)previous);
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

  (void)fprintf(stderr, "FAIL %s: bytes differ\n", label);
  return 1;
}

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x601u,
  "test/request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x602u,
  "test/response",
  0u,
  1u
};

static const grcl_type_support_t k_other_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x603u,
  "test/other",
  0u,
  1u
};

static grcl_runtime_options_t inprocess_runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "service-client-test";
  options.profile_name = "native-inprocess";
  return options;
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/test";
  return options;
}

static grcl_service_options_t service_options(
  const char * service_name,
  const grcl_type_support_t * request_type,
  const grcl_type_support_t * response_type)
{
  grcl_service_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
  options.request_type_support = request_type;
  options.response_type_support = response_type;
  return options;
}

static grcl_client_options_t client_options(
  const char * service_name,
  const grcl_type_support_t * request_type,
  const grcl_type_support_t * response_type)
{
  grcl_client_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
  options.request_type_support = request_type;
  options.response_type_support = response_type;
  return options;
}

static grcl_executor_options_t executor_options(void)
{
  grcl_executor_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static int create_inprocess_service_client(
  grcl_runtime_t ** runtime,
  grcl_node_t ** service_node,
  grcl_node_t ** client_node,
  grcl_service_t ** service,
  grcl_client_t ** client,
  grcl_executor_t ** executor)
{
  grcl_runtime_options_t runtime_create_options = inprocess_runtime_options();
  grcl_node_options_t service_node_create_options = node_options("server");
  grcl_node_options_t client_node_create_options = node_options("client");
  grcl_service_options_t service_create_options =
    service_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_client_options_t client_create_options =
    client_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_executor_options_t executor_create_options = executor_options();

  *runtime = NULL;
  *service_node = NULL;
  *client_node = NULL;
  *service = NULL;
  *client = NULL;
  *executor = NULL;

  if (grcl_runtime_create(&runtime_create_options, runtime) != GRCL_OK ||
    grcl_node_create(*runtime, &service_node_create_options, service_node) != GRCL_OK ||
    grcl_node_create(*runtime, &client_node_create_options, client_node) != GRCL_OK ||
    grcl_service_create(*service_node, &service_create_options, service) != GRCL_OK ||
    grcl_client_create(*client_node, &client_create_options, client) != GRCL_OK ||
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK) {
    (void)fprintf(stderr, "FAIL create native in-process service/client fixture\n");
    (void)grcl_runtime_destroy(*runtime);
    *service_node = NULL;
    *client_node = NULL;
    *service = NULL;
    *client = NULL;
    *executor = NULL;
    return 1;
  }

  return 0;
}

static int test_null_default_runtime_keeps_service_client_unsupported(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_service_options_t service_create_options =
    service_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_client_options_t client_create_options =
    client_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {1u, 2u, 3u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_service_create(node, &service_create_options, &service) != GRCL_OK ||
    grcl_client_create(node, &client_create_options, &client) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "null send request unsupported",
      grcl_client_send_request_bytes(client, payload, sizeof(payload), &request_id),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null service take unsupported",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &request_id),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null service response unsupported",
      grcl_service_send_response_bytes(service, 1u, payload, sizeof(payload)),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null client take unsupported",
      grcl_client_take_response_bytes(
        client,
        1u,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("stop null runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_request_response_round_trip_and_correlation(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  unsigned char request[] = {10u, 20u, 30u, 40u};
  const unsigned char expected_request[] = {10u, 20u, 30u, 40u};
  unsigned char response[] = {50u, 60u, 70u};
  const unsigned char expected_response[] = {50u, 60u, 70u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t sent_id = 0u;
  grcl_request_id_t taken_id = 0u;

  if (create_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0 ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_executor_add_node(executor, client_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "take before request dispatch",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "send request",
      grcl_client_send_request_bytes(client, request, sizeof(request), &sent_id),
      GRCL_OK) != 0 ||
    expect_nonzero_request_id("request id non-zero", sent_id) != 0 ||
    expect_result(
      "response not ready before dispatch",
      grcl_client_take_response_bytes(
        client,
        sent_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  memset(request, 0xff, sizeof(request));

  if (expect_result("request spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take dispatched request",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_OK) != 0 ||
    expect_same_request_id("request id preserved", taken_id, sent_id) != 0 ||
    expect_size("request size", receive_size, sizeof(expected_request)) != 0 ||
    expect_bytes(
      "request payload copied",
      receive,
      expected_request,
      sizeof(expected_request)) != 0 ||
    expect_result(
      "send response",
      grcl_service_send_response_bytes(
        service,
        taken_id,
        response,
        sizeof(response)),
      GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  memset(response, 0xee, sizeof(response));
  memset(receive, 0u, sizeof(receive));

  if (expect_result(
      "response still not ready before response spin",
      grcl_client_take_response_bytes(
        client,
        sent_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("response spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "wrong response id not returned",
      grcl_client_take_response_bytes(
        client,
        sent_id + 1u,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NOT_FOUND) != 0 ||
    expect_result(
      "take response",
      grcl_client_take_response_bytes(
        client,
        sent_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_size("response size", receive_size, sizeof(expected_response)) != 0 ||
    expect_bytes(
      "response payload copied",
      receive,
      expected_response,
      sizeof(expected_response)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_missing_service_and_type_mismatch(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * missing_client = NULL;
  grcl_client_t * mismatch_client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = inprocess_runtime_options();
  grcl_node_options_t service_node_create_options = node_options("server");
  grcl_node_options_t client_node_create_options = node_options("client");
  grcl_service_options_t service_create_options =
    service_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_client_options_t missing_client_options =
    client_options("svc.missing", &k_request_type, &k_response_type);
  grcl_client_options_t mismatch_client_options =
    client_options("svc.alpha", &k_other_type, &k_response_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {1u};
  grcl_request_id_t request_id = 0u;

  if (grcl_runtime_create(&runtime_create_options, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &service_node_create_options, &service_node) != GRCL_OK ||
    grcl_node_create(runtime, &client_node_create_options, &client_node) != GRCL_OK ||
    grcl_service_create(service_node, &service_create_options, &service) != GRCL_OK ||
    grcl_client_create(client_node, &missing_client_options, &missing_client) != GRCL_OK ||
    grcl_client_create(client_node, &mismatch_client_options, &mismatch_client) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_executor_add_node(executor, client_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)service;
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "missing service",
      grcl_client_send_request_bytes(
        missing_client,
        payload,
        sizeof(payload),
        &request_id),
      GRCL_ERROR_PEER_UNAVAILABLE) != 0 ||
    expect_result(
      "service type mismatch",
      grcl_client_send_request_bytes(
        mismatch_client,
        payload,
        sizeof(payload),
        &request_id),
      GRCL_ERROR_TYPE_MISMATCH) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_small_buffers_do_not_consume_request_or_response(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char request[] = {1u, 2u, 3u, 4u};
  const unsigned char response[] = {5u, 6u, 7u};
  unsigned char small_receive[2u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;
  grcl_request_id_t taken_id = 0u;

  if (create_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0 ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_executor_add_node(executor, client_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "send request",
      grcl_client_send_request_bytes(client, request, sizeof(request), &request_id),
      GRCL_OK) != 0 ||
    expect_result("request spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small request take",
      grcl_service_take_request_bytes(
        service,
        small_receive,
        sizeof(small_receive),
        &receive_size,
        &taken_id),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small request required size", receive_size, sizeof(request)) != 0 ||
    expect_result(
      "later request take succeeds",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_OK) != 0 ||
    expect_same_request_id("later request id", taken_id, request_id) != 0 ||
    expect_bytes("later request bytes", receive, request, sizeof(request)) != 0 ||
    expect_result(
      "send response",
      grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
      GRCL_OK) != 0 ||
    expect_result("response spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small response take",
      grcl_client_take_response_bytes(
        client,
        request_id,
        small_receive,
        sizeof(small_receive),
        &receive_size),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small response required size", receive_size, sizeof(response)) != 0 ||
    expect_result(
      "later response take succeeds",
      grcl_client_take_response_bytes(
        client,
        request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_bytes("later response bytes", receive, response, sizeof(response)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_unknown_request_ids_and_payload_limits(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {1u};
  unsigned char oversized_payload[1025u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;

  if (create_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0 ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_executor_add_node(executor, client_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "unknown service request id",
      grcl_service_send_response_bytes(service, 99u, payload, sizeof(payload)),
      GRCL_ERROR_NOT_FOUND) != 0 ||
    expect_result(
      "unknown client request id",
      grcl_client_take_response_bytes(
        client,
        99u,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NOT_FOUND) != 0 ||
    expect_result(
      "oversized request",
      grcl_client_send_request_bytes(
        client,
        oversized_payload,
        sizeof(oversized_payload),
        &request_id),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0 ||
    expect_result(
      "oversized response unknown id",
      grcl_service_send_response_bytes(
        service,
        99u,
        oversized_payload,
        sizeof(oversized_payload)),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_queue_saturation_and_executor_membership_bounds(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char request[] = {1u, 2u};
  const unsigned char response[] = {3u, 4u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_ids[9u] = {0};
  grcl_request_id_t taken_id = 0u;

  if (create_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0 ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "send request while client outside executor",
      grcl_client_send_request_bytes(client, request, sizeof(request), &request_ids[0u]),
      GRCL_OK) != 0 ||
    expect_result("spin without client node", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "service still has no data without client executor membership",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("add client node", grcl_executor_add_node(executor, client_node), GRCL_OK) != 0 ||
    expect_result("dispatch first request", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take first request",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_OK) != 0 ||
    expect_result("remove service node", grcl_executor_remove_node(executor, service_node), GRCL_OK) != 0 ||
    expect_result(
      "send response while service outside executor",
      grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
      GRCL_OK) != 0 ||
    expect_result("spin without service node", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "client still has no response without service executor membership",
      grcl_client_take_response_bytes(
        client,
        taken_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("add service node again", grcl_executor_add_node(executor, service_node), GRCL_OK) != 0 ||
    expect_result("dispatch first response", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take first response",
      grcl_client_take_response_bytes(
        client,
        taken_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < 8u; ++i) {
    if (expect_result(
        "fill pending request queue",
        grcl_client_send_request_bytes(
          client,
          request,
          sizeof(request),
          &request_ids[i]),
        GRCL_OK) != 0 ||
      (i > 0u &&
      expect_request_id_greater(
        "monotonic request id",
        request_ids[i],
        request_ids[i - 1u]) != 0)) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "request queue saturated",
      grcl_client_send_request_bytes(
        client,
        request,
        sizeof(request),
        &request_ids[8u]),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < 8u; ++i) {
    if (expect_result(
        "dispatch one pending request",
        grcl_executor_spin_once(executor, 0u),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  for (size_t i = 0u; i < 8u; ++i) {
    if (expect_result(
        "take queued request",
        grcl_service_take_request_bytes(
          service,
          receive,
          sizeof(receive),
          &receive_size,
          &taken_id),
        GRCL_OK) != 0 ||
      expect_result(
        "queue response",
        grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  for (size_t i = 0u; i < 8u; ++i) {
    if (expect_result(
        "dispatch one pending response",
        grcl_executor_spin_once(executor, 0u),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "send request after response queue filled",
      grcl_client_send_request_bytes(
        client,
        request,
        sizeof(request),
        &request_ids[8u]),
      GRCL_OK) != 0 ||
    expect_result("dispatch one more request", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take one more request",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_id),
      GRCL_OK) != 0 ||
    expect_result(
      "response queue saturated",
      grcl_service_send_response_bytes(service, taken_id, response, sizeof(response)),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_service_client_operations_require_started_runtime(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {1u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;

  if (create_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0 ||
    grcl_executor_add_node(executor, service_node) != GRCL_OK ||
    grcl_executor_add_node(executor, client_node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "send before start",
      grcl_client_send_request_bytes(client, payload, sizeof(payload), &request_id),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "service take before start",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &request_id),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "service response before start",
      grcl_service_send_response_bytes(service, 1u, payload, sizeof(payload)),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "client take before start",
      grcl_client_take_response_bytes(
        client,
        1u,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "spin before start",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  if (test_null_default_runtime_keeps_service_client_unsupported() != 0) {
    return 1;
  }
  if (test_request_response_round_trip_and_correlation() != 0) {
    return 1;
  }
  if (test_missing_service_and_type_mismatch() != 0) {
    return 1;
  }
  if (test_small_buffers_do_not_consume_request_or_response() != 0) {
    return 1;
  }
  if (test_unknown_request_ids_and_payload_limits() != 0) {
    return 1;
  }
  if (test_queue_saturation_and_executor_membership_bounds() != 0) {
    return 1;
  }
  if (test_service_client_operations_require_started_runtime() != 0) {
    return 1;
  }

  return 0;
}
