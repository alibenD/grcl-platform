#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/version.h>

#define INPROCESS_MAX_PAYLOAD_BYTES 1024u
#define INPROCESS_QUEUE_CAPACITY 8u

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

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x701u,
  "m4/message",
  0u,
  1u
};

static const grcl_type_support_t k_other_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x702u,
  "m4/message/other",
  0u,
  1u
};

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x703u,
  "m4/request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x704u,
  "m4/response",
  0u,
  1u
};

static const grcl_type_support_t k_other_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x705u,
  "m4/request/other",
  0u,
  1u
};

static grcl_runtime_options_t inprocess_runtime_options(const char * name)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = name;
  options.profile_name = "native-inprocess";
  return options;
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/m4";
  return options;
}

static grcl_publisher_options_t publisher_options(
  const char * topic_name,
  const grcl_type_support_t * type_support)
{
  grcl_publisher_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = topic_name;
  options.type_support = type_support;
  return options;
}

static grcl_subscription_options_t subscription_options(
  const char * topic_name,
  const grcl_type_support_t * type_support)
{
  grcl_subscription_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = topic_name;
  options.type_support = type_support;
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

static int create_started_null_runtime_with_pub_sub(
  grcl_runtime_t ** runtime,
  grcl_node_t ** node,
  grcl_publisher_t ** publisher,
  grcl_subscription_t ** subscription,
  grcl_executor_t ** executor)
{
  grcl_node_options_t node_create_options = node_options("null-node");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  *runtime = NULL;
  *node = NULL;
  *publisher = NULL;
  *subscription = NULL;
  *executor = NULL;

  if (grcl_runtime_create(NULL, runtime) != GRCL_OK ||
    grcl_node_create(*runtime, &node_create_options, node) != GRCL_OK ||
    grcl_publisher_create(*node, &publisher_create_options, publisher) != GRCL_OK ||
    grcl_subscription_create(*node, &subscription_create_options, subscription) != GRCL_OK ||
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK ||
    grcl_executor_add_node(*executor, *node) != GRCL_OK ||
    grcl_runtime_start(*runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(*runtime);
    *runtime = NULL;
    *node = NULL;
    *publisher = NULL;
    *subscription = NULL;
    *executor = NULL;
    return 1;
  }

  return 0;
}

static int create_started_inprocess_pub_sub(
  grcl_runtime_t ** runtime,
  grcl_node_t ** node,
  grcl_publisher_t ** publisher,
  grcl_subscription_t ** subscription,
  grcl_executor_t ** executor)
{
  grcl_runtime_options_t runtime_create_options =
    inprocess_runtime_options("m4-pub-sub-runtime");
  grcl_node_options_t node_create_options = node_options("pub-sub-node");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  *runtime = NULL;
  *node = NULL;
  *publisher = NULL;
  *subscription = NULL;
  *executor = NULL;

  if (grcl_runtime_create(&runtime_create_options, runtime) != GRCL_OK ||
    grcl_node_create(*runtime, &node_create_options, node) != GRCL_OK ||
    grcl_publisher_create(*node, &publisher_create_options, publisher) != GRCL_OK ||
    grcl_subscription_create(*node, &subscription_create_options, subscription) != GRCL_OK ||
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK ||
    grcl_executor_add_node(*executor, *node) != GRCL_OK ||
    grcl_runtime_start(*runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(*runtime);
    *runtime = NULL;
    *node = NULL;
    *publisher = NULL;
    *subscription = NULL;
    *executor = NULL;
    return 1;
  }

  return 0;
}

static int create_started_inprocess_service_client(
  grcl_runtime_t ** runtime,
  grcl_node_t ** service_node,
  grcl_node_t ** client_node,
  grcl_service_t ** service,
  grcl_client_t ** client,
  grcl_executor_t ** executor)
{
  grcl_runtime_options_t runtime_create_options =
    inprocess_runtime_options("m4-service-client-runtime");
  grcl_node_options_t service_node_create_options = node_options("service-node");
  grcl_node_options_t client_node_create_options = node_options("client-node");
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
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK ||
    grcl_executor_add_node(*executor, *service_node) != GRCL_OK ||
    grcl_executor_add_node(*executor, *client_node) != GRCL_OK ||
    grcl_runtime_start(*runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(*runtime);
    *runtime = NULL;
    *service_node = NULL;
    *client_node = NULL;
    *service = NULL;
    *client = NULL;
    *executor = NULL;
    return 1;
  }

  return 0;
}

static int create_started_null_runtime_with_service_client(
  grcl_runtime_t ** runtime,
  grcl_node_t ** node,
  grcl_service_t ** service,
  grcl_client_t ** client,
  grcl_executor_t ** executor)
{
  grcl_node_options_t node_create_options = node_options("null-service-node");
  grcl_service_options_t service_create_options =
    service_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_client_options_t client_create_options =
    client_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_executor_options_t executor_create_options = executor_options();

  *runtime = NULL;
  *node = NULL;
  *service = NULL;
  *client = NULL;
  *executor = NULL;

  if (grcl_runtime_create(NULL, runtime) != GRCL_OK ||
    grcl_node_create(*runtime, &node_create_options, node) != GRCL_OK ||
    grcl_service_create(*node, &service_create_options, service) != GRCL_OK ||
    grcl_client_create(*node, &client_create_options, client) != GRCL_OK ||
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK ||
    grcl_executor_add_node(*executor, *node) != GRCL_OK ||
    grcl_runtime_start(*runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(*runtime);
    *runtime = NULL;
    *node = NULL;
    *service = NULL;
    *client = NULL;
    *executor = NULL;
    return 1;
  }

  return 0;
}

static int test_null_default_backend_keeps_messaging_and_executor_unsupported(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {1u, 2u, 3u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;

  if (create_started_null_runtime_with_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "null publish unsupported",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null take unsupported",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null spin unsupported",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("stop null runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0 ||
    expect_result("destroy null runtime", grcl_runtime_destroy(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (create_started_null_runtime_with_service_client(
      &runtime,
      &node,
      &service,
      &client,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "null send request unsupported",
      grcl_client_send_request_bytes(client, payload, sizeof(payload), &request_id),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null take request unsupported",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &request_id),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null send response unsupported",
      grcl_service_send_response_bytes(service, 1u, payload, sizeof(payload)),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null take response unsupported",
      grcl_client_take_response_bytes(
        client,
        1u,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null service/client spin unsupported",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("stop null service runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null service runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_pub_sub_requires_spin_and_delivers_one_message_per_pass_fifo(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_subscription_t * topic_mismatch_subscription = NULL;
  grcl_subscription_t * type_mismatch_subscription = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options =
    inprocess_runtime_options("m4-bounded-pub-sub-runtime");
  grcl_node_options_t node_create_options = node_options("bounded-node");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t topic_mismatch_options =
    subscription_options("topic.other", &k_message_type);
  grcl_subscription_options_t type_mismatch_options =
    subscription_options("topic.alpha", &k_other_message_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char first_payload[] = {0x11u, 0x12u, 0x13u};
  const unsigned char second_payload[] = {0x21u, 0x22u, 0x23u, 0x24u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (grcl_runtime_create(&runtime_create_options, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK ||
    grcl_subscription_create(node, &subscription_create_options, &subscription) != GRCL_OK ||
    grcl_subscription_create(node, &topic_mismatch_options, &topic_mismatch_subscription) != GRCL_OK ||
    grcl_subscription_create(node, &type_mismatch_options, &type_mismatch_subscription) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "take before spin",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "first publish",
      grcl_publisher_publish_bytes(publisher, first_payload, sizeof(first_payload)),
      GRCL_OK) != 0 ||
    expect_result(
      "second publish",
      grcl_publisher_publish_bytes(publisher, second_payload, sizeof(second_payload)),
      GRCL_OK) != 0 ||
    expect_result("first bounded spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "first matching take",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_OK) != 0 ||
    expect_size("first matching size", receive_size, sizeof(first_payload)) != 0 ||
    expect_bytes("first matching bytes", receive, first_payload, sizeof(first_payload)) != 0 ||
    expect_result(
      "bounded pass leaves second message pending",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "topic mismatch has no data",
      grcl_subscription_take_bytes(
        topic_mismatch_subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "type mismatch has no data",
      grcl_subscription_take_bytes(
        type_mismatch_subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("second bounded spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "second matching take",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_OK) != 0 ||
    expect_size("second matching size", receive_size, sizeof(second_payload)) != 0 ||
    expect_bytes("second matching bytes", receive, second_payload, sizeof(second_payload)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_pub_sub_payload_limits_and_queue_saturation(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  unsigned char payload[4u] = {0u, 1u, 2u, 3u};
  unsigned char oversized_payload[INPROCESS_MAX_PAYLOAD_BYTES + 1u] = {0};

  if (create_started_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "oversized publish",
      grcl_publisher_publish_bytes(publisher, oversized_payload, sizeof(oversized_payload)),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    payload[0u] = (unsigned char)i;
    if (expect_result(
        "fill publisher pending queue",
        grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "publisher pending saturated",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    if (expect_result("fill subscription ready queue", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "publish after pending drained",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result(
      "subscription ready saturated",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_pub_sub_small_take_buffer_does_not_consume_message(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {0xa1u, 0xa2u, 0xa3u, 0xa4u};
  unsigned char small_receive[2u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (create_started_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "publish for small buffer test",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result("dispatch for small buffer test", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small subscription buffer",
      grcl_subscription_take_bytes(
        subscription,
        small_receive,
        sizeof(small_receive),
        &receive_size),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small subscription required size", receive_size, sizeof(payload)) != 0 ||
    expect_result(
      "message still available after capacity error",
      grcl_subscription_take_bytes(subscription, receive, sizeof(receive), &receive_size),
      GRCL_OK) != 0 ||
    expect_bytes("re-take bytes", receive, payload, sizeof(payload)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_service_client_requires_spin_and_preserves_request_correlation(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char first_request[] = {0x31u, 0x32u};
  const unsigned char second_request[] = {0x41u, 0x42u, 0x43u};
  const unsigned char response_payload[] = {0x51u, 0x52u, 0x53u, 0x54u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t first_request_id = 0u;
  grcl_request_id_t second_request_id = 0u;
  grcl_request_id_t taken_request_id = 0u;

  if (create_started_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "no request before spin",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "send first request",
      grcl_client_send_request_bytes(client, first_request, sizeof(first_request), &first_request_id),
      GRCL_OK) != 0 ||
    expect_nonzero_request_id("first request id non-zero", first_request_id) != 0 ||
    expect_result(
      "send second request",
      grcl_client_send_request_bytes(client, second_request, sizeof(second_request), &second_request_id),
      GRCL_OK) != 0 ||
    expect_nonzero_request_id("second request id non-zero", second_request_id) != 0 ||
    expect_result(
      "response not ready before request dispatch",
      grcl_client_take_response_bytes(
        client,
        first_request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("first request spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take first request only",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_OK) != 0 ||
    expect_same_request_id("first request id preserved", taken_request_id, first_request_id) != 0 ||
    expect_bytes("first request bytes", receive, first_request, sizeof(first_request)) != 0 ||
    expect_result(
      "second request still pending after one spin",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "unknown request id before take",
      grcl_service_send_response_bytes(service, second_request_id, response_payload, sizeof(response_payload)),
      GRCL_ERROR_NOT_FOUND) != 0 ||
    expect_result(
      "send first response",
      grcl_service_send_response_bytes(
        service,
        first_request_id,
        response_payload,
        sizeof(response_payload)),
      GRCL_OK) != 0 ||
    expect_result(
      "response still pending before response spin",
      grcl_client_take_response_bytes(
        client,
        first_request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("second request spin", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take second request after second spin",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_OK) != 0 ||
    expect_same_request_id("second request id preserved", taken_request_id, second_request_id) != 0 ||
    expect_bytes("second request bytes", receive, second_request, sizeof(second_request)) != 0 ||
    expect_result(
      "take correlated response",
      grcl_client_take_response_bytes(
        client,
        first_request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_size("response size", receive_size, sizeof(response_payload)) != 0 ||
    expect_bytes("response bytes", receive, response_payload, sizeof(response_payload)) != 0 ||
    expect_result(
      "second response still absent",
      grcl_client_take_response_bytes(
        client,
        second_request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_service_client_missing_service_and_type_mismatch(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * missing_client = NULL;
  grcl_client_t * mismatch_client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options =
    inprocess_runtime_options("m4-service-mismatch-runtime");
  grcl_node_options_t service_node_create_options = node_options("service-node");
  grcl_node_options_t client_node_create_options = node_options("client-node");
  grcl_service_options_t service_create_options =
    service_options("svc.alpha", &k_request_type, &k_response_type);
  grcl_client_options_t missing_client_options =
    client_options("svc.missing", &k_request_type, &k_response_type);
  grcl_client_options_t mismatch_client_options =
    client_options("svc.alpha", &k_other_request_type, &k_response_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {0x61u};
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
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "missing service",
      grcl_client_send_request_bytes(missing_client, payload, sizeof(payload), &request_id),
      GRCL_ERROR_PEER_UNAVAILABLE) != 0 ||
    expect_result(
      "service type mismatch",
      grcl_client_send_request_bytes(mismatch_client, payload, sizeof(payload), &request_id),
      GRCL_ERROR_TYPE_MISMATCH) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_service_client_small_buffers_do_not_consume_messages(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char request_payload[] = {0x71u, 0x72u, 0x73u, 0x74u};
  const unsigned char response_payload[] = {0x81u, 0x82u, 0x83u};
  unsigned char small_receive[2u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_id = 0u;
  grcl_request_id_t taken_request_id = 0u;

  if (create_started_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "send request for small buffer test",
      grcl_client_send_request_bytes(
        client,
        request_payload,
        sizeof(request_payload),
        &request_id),
      GRCL_OK) != 0 ||
    expect_result("spin request for small buffer test", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small request buffer",
      grcl_service_take_request_bytes(
        service,
        small_receive,
        sizeof(small_receive),
        &receive_size,
        &taken_request_id),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small request required size", receive_size, sizeof(request_payload)) != 0 ||
    expect_result(
      "request still available after capacity error",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_OK) != 0 ||
    expect_same_request_id("request id after re-take", taken_request_id, request_id) != 0 ||
    expect_bytes("request bytes after re-take", receive, request_payload, sizeof(request_payload)) != 0 ||
    expect_result(
      "send response for small buffer test",
      grcl_service_send_response_bytes(
        service,
        request_id,
        response_payload,
        sizeof(response_payload)),
      GRCL_OK) != 0 ||
    expect_result("spin response for small buffer test", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small response buffer",
      grcl_client_take_response_bytes(
        client,
        request_id,
        small_receive,
        sizeof(small_receive),
        &receive_size),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small response required size", receive_size, sizeof(response_payload)) != 0 ||
    expect_result(
      "response still available after capacity error",
      grcl_client_take_response_bytes(
        client,
        request_id,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_bytes("response bytes after re-take", receive, response_payload, sizeof(response_payload)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_service_client_payload_limits_and_queue_saturation(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * service_node = NULL;
  grcl_node_t * client_node = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  unsigned char request_payload[3u] = {0x91u, 0x92u, 0x93u};
  unsigned char response_payload[2u] = {0xa1u, 0xa2u};
  unsigned char oversized_payload[INPROCESS_MAX_PAYLOAD_BYTES + 1u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;
  grcl_request_id_t request_ids[INPROCESS_QUEUE_CAPACITY + 1u] = {0};
  grcl_request_id_t taken_request_id = 0u;

  if (create_started_inprocess_service_client(
      &runtime,
      &service_node,
      &client_node,
      &service,
      &client,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "oversized request payload",
      grcl_client_send_request_bytes(client, oversized_payload, sizeof(oversized_payload), &request_ids[0u]),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0 ||
    expect_result(
      "oversized response payload",
      grcl_service_send_response_bytes(service, 1u, oversized_payload, sizeof(oversized_payload)),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    request_payload[0u] = (unsigned char)(0xb0u + i);
    if (expect_result(
        "fill pending request queue",
        grcl_client_send_request_bytes(
          client,
          request_payload,
          sizeof(request_payload),
          &request_ids[i]),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "pending request queue saturated",
      grcl_client_send_request_bytes(
        client,
        request_payload,
        sizeof(request_payload),
        &request_ids[INPROCESS_QUEUE_CAPACITY]),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    if (expect_result("fill ready request queue", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "queue one more request after pending drain",
      grcl_client_send_request_bytes(
        client,
        request_payload,
        sizeof(request_payload),
        &request_ids[INPROCESS_QUEUE_CAPACITY]),
      GRCL_OK) != 0 ||
    expect_result(
      "ready request queue saturated",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    if (expect_result(
        "take queued request",
        grcl_service_take_request_bytes(
          service,
          receive,
          sizeof(receive),
          &receive_size,
          &taken_request_id),
        GRCL_OK) != 0 ||
      expect_result(
        "queue response",
        grcl_service_send_response_bytes(
          service,
          taken_request_id,
          response_payload,
          sizeof(response_payload)),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "queue one more request for response saturation",
      grcl_client_send_request_bytes(
        client,
        request_payload,
        sizeof(request_payload),
        &request_ids[INPROCESS_QUEUE_CAPACITY]),
      GRCL_OK) != 0 ||
    expect_result("dispatch queued request", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take queued request for response saturation",
      grcl_service_take_request_bytes(
        service,
        receive,
        sizeof(receive),
        &receive_size,
        &taken_request_id),
      GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < INPROCESS_QUEUE_CAPACITY; ++i) {
    if (expect_result("fill ready response queue", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "response ready queue saturated",
      grcl_service_send_response_bytes(
        service,
        taken_request_id,
        response_payload,
        sizeof(response_payload)),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  if (test_null_default_backend_keeps_messaging_and_executor_unsupported() != 0) {
    return 1;
  }
  if (test_pub_sub_requires_spin_and_delivers_one_message_per_pass_fifo() != 0) {
    return 1;
  }
  if (test_pub_sub_payload_limits_and_queue_saturation() != 0) {
    return 1;
  }
  if (test_pub_sub_small_take_buffer_does_not_consume_message() != 0) {
    return 1;
  }
  if (test_service_client_requires_spin_and_preserves_request_correlation() != 0) {
    return 1;
  }
  if (test_service_client_missing_service_and_type_mismatch() != 0) {
    return 1;
  }
  if (test_service_client_small_buffers_do_not_consume_messages() != 0) {
    return 1;
  }
  if (test_service_client_payload_limits_and_queue_saturation() != 0) {
    return 1;
  }

  return 0;
}
