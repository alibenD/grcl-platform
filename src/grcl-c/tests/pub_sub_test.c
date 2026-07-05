#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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

  (void)label;
  return 1;
}

static int expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)label;
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

  (void)label;
  return 1;
}

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x501u,
  "test/message",
  0u,
  1u
};

static const grcl_type_support_t k_other_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x502u,
  "test/other",
  0u,
  1u
};

static sigjmp_buf g_spin_guard_jump;
static volatile sig_atomic_t g_spin_guard_timed_out = 0;

static void handle_spin_guard_alarm(int signum)
{
  (void)signum;
  g_spin_guard_timed_out = 1;
  siglongjmp(g_spin_guard_jump, 1);
}

static grcl_result_t guarded_spin_once(
  grcl_executor_t * executor,
  uint64_t timeout_ns,
  unsigned int guard_seconds)
{
  void (* previous_handler)(int);

  g_spin_guard_timed_out = 0;
  previous_handler = signal(SIGALRM, handle_spin_guard_alarm);
  if (previous_handler == SIG_ERR) {
    return GRCL_ERROR_INTERRUPTED;
  }

  if (sigsetjmp(g_spin_guard_jump, 1) != 0) {
    (void)alarm(0u);
    (void)signal(SIGALRM, previous_handler);
    return GRCL_ERROR_INTERRUPTED;
  }

  (void)alarm(guard_seconds);
  {
    grcl_result_t result = grcl_executor_spin_once(executor, timeout_ns);
    (void)alarm(0u);
    (void)signal(SIGALRM, previous_handler);
    return result;
  }
}

static grcl_runtime_options_t inprocess_runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "pub-sub-test";
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

static grcl_executor_options_t executor_options(void)
{
  grcl_executor_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static int create_inprocess_pub_sub(
  grcl_runtime_t ** runtime,
  grcl_node_t ** node,
  grcl_publisher_t ** publisher,
  grcl_subscription_t ** subscription,
  grcl_executor_t ** executor)
{
  grcl_runtime_options_t runtime_create_options = inprocess_runtime_options();
  grcl_node_options_t node_create_options = node_options("alpha");
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
    grcl_subscription_create(
      *node,
      &subscription_create_options,
      subscription) != GRCL_OK ||
    grcl_executor_create(*runtime, &executor_create_options, executor) != GRCL_OK ||
    grcl_executor_add_node(*executor, *node) != GRCL_OK) {
    (void)grcl_runtime_destroy(*runtime);
    return 1;
  }

  return 0;
}

static int test_null_default_runtime_keeps_pub_sub_unsupported(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {1u, 2u, 3u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK ||
    grcl_subscription_create(node, &subscription_create_options, &subscription) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "null publish unsupported",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null take unsupported",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result(
      "null spin unsupported",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("stop null runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy null runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_pub_sub_copies_payload_and_delivers_after_spin(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  unsigned char payload[] = {10u, 20u, 30u, 40u};
  const unsigned char expected[] = {10u, 20u, 30u, 40u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (create_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0) {
    return 1;
  }

  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "take before spin has no data",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "publish copied payload",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  memset(payload, 0xff, sizeof(payload));

  if (expect_result(
      "spin dispatches payload",
      grcl_executor_spin_once(executor, 0u),
      GRCL_OK) != 0 ||
    expect_result(
      "take dispatched payload",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_size("received payload size", receive_size, sizeof(expected)) != 0 ||
    expect_bytes("received copied payload", receive, expected, sizeof(expected)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_type_mismatch_produces_no_delivery(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * mismatch_subscription = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = inprocess_runtime_options();
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t mismatch_subscription_options =
    subscription_options("topic.alpha", &k_other_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {7u, 8u, 9u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (grcl_runtime_create(&runtime_create_options, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK ||
    grcl_subscription_create(
      node,
      &mismatch_subscription_options,
      &mismatch_subscription) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "publish with only mismatched subscription",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result("spin no compatible subscription", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "mismatched take has no data",
      grcl_subscription_take_bytes(
        mismatch_subscription,
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

static int test_non_member_publisher_spin_returns_without_consuming_message(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * publisher_node = NULL;
  grcl_node_t * subscription_node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = inprocess_runtime_options();
  grcl_node_options_t publisher_node_options = node_options("publisher");
  grcl_node_options_t subscription_node_options = node_options("subscription");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {4u, 5u, 6u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (grcl_runtime_create(&runtime_create_options, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &publisher_node_options, &publisher_node) != GRCL_OK ||
    grcl_node_create(runtime, &subscription_node_options, &subscription_node) != GRCL_OK ||
    grcl_publisher_create(
      publisher_node,
      &publisher_create_options,
      &publisher) != GRCL_OK ||
    grcl_subscription_create(
      subscription_node,
      &subscription_create_options,
      &subscription) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, subscription_node) != GRCL_OK ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "publish pending message from non-member publisher node",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result(
      "guarded spin returns for non-member publisher",
      guarded_spin_once(executor, 0u, 1u),
      GRCL_OK) != 0 ||
    expect_result(
      "first take has no data",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result("add publisher node later", grcl_executor_add_node(executor, publisher_node), GRCL_OK) != 0 ||
    expect_result("later spin dispatches original message", guarded_spin_once(executor, 0u, 1u), GRCL_OK) != 0 ||
    expect_result(
      "later take succeeds with original message",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_size("later take size", receive_size, sizeof(payload)) != 0 ||
    expect_bytes("later take bytes", receive, payload, sizeof(payload)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_small_receive_buffer_does_not_consume_message(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {1u, 3u, 5u, 7u};
  unsigned char small_receive[2u] = {0};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (create_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0 ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "publish payload",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result("spin payload", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "small take reports capacity",
      grcl_subscription_take_bytes(
        subscription,
        small_receive,
        sizeof(small_receive),
        &receive_size),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_size("small take required size", receive_size, sizeof(payload)) != 0 ||
    expect_result(
      "later take succeeds",
      grcl_subscription_take_bytes(
        subscription,
        receive,
        sizeof(receive),
        &receive_size),
      GRCL_OK) != 0 ||
    expect_size("later take size", receive_size, sizeof(payload)) != 0 ||
    expect_bytes("later take bytes", receive, payload, sizeof(payload)) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_payload_and_queue_capacity_errors(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  unsigned char payload[4u] = {0};
  unsigned char oversized_payload[1025u] = {0};

  if (create_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0 ||
    grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "oversized payload",
      grcl_publisher_publish_bytes(
        publisher,
        oversized_payload,
        sizeof(oversized_payload)),
      GRCL_ERROR_PAYLOAD_TOO_LARGE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  for (size_t i = 0u; i < 8u; ++i) {
    payload[0u] = (unsigned char)i;
    if (expect_result(
        "fill publish queue",
        grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
        GRCL_OK) != 0) {
      (void)grcl_runtime_stop(runtime);
      (void)grcl_runtime_destroy(runtime);
      return 1;
    }
  }

  if (expect_result(
      "publish queue saturated",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("spin fills subscription queue", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "publish after pending queue drained",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result(
      "subscription ready queue saturated",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result("stop runtime", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_pub_sub_operations_require_started_runtime(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  const unsigned char payload[] = {1u};
  unsigned char receive[8u] = {0};
  size_t receive_size = 0u;

  if (create_inprocess_pub_sub(
      &runtime,
      &node,
      &publisher,
      &subscription,
      &executor) != 0) {
    return 1;
  }

  if (expect_result(
      "publish before start",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "take before start",
      grcl_subscription_take_bytes(
        subscription,
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
  int failures = 0;

  failures += test_null_default_runtime_keeps_pub_sub_unsupported();
  failures += test_pub_sub_copies_payload_and_delivers_after_spin();
  failures += test_type_mismatch_produces_no_delivery();
  failures += test_non_member_publisher_spin_returns_without_consuming_message();
  failures += test_small_receive_buffer_does_not_consume_message();
  failures += test_payload_and_queue_capacity_errors();
  failures += test_pub_sub_operations_require_started_runtime();

  return failures == 0 ? 0 : 1;
}
