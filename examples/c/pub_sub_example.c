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

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x9001u,
  "example/message",
  0u,
  1u
};

static grcl_runtime_options_t runtime_options(void)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "pub-sub-example";
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
  options.topic_name = "examples/topic.alpha";
  options.type_support = &k_message_type;
  return options;
}

static grcl_subscription_options_t subscription_options(void)
{
  grcl_subscription_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "examples/topic.alpha";
  options.type_support = &k_message_type;
  return options;
}

static grcl_executor_options_t executor_options(void)
{
  grcl_executor_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

int main(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * publisher_node = NULL;
  grcl_node_t * subscription_node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t publisher_node_options = node_options("talker");
  grcl_node_options_t subscription_node_options = node_options("listener");
  grcl_publisher_options_t publisher_create_options = publisher_options();
  grcl_subscription_options_t subscription_create_options = subscription_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {0x10u, 0x20u, 0x30u, 0x40u};
  unsigned char received[16] = {0};
  size_t received_size = 0u;
  int started = 0;
  int failures = 0;

  if (expect_result(
      "create runtime",
      grcl_runtime_create(&runtime_create_options, &runtime),
      GRCL_OK) != 0 ||
    expect_result(
      "create publisher node",
      grcl_node_create(runtime, &publisher_node_options, &publisher_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create subscription node",
      grcl_node_create(runtime, &subscription_node_options, &subscription_node),
      GRCL_OK) != 0 ||
    expect_result(
      "create publisher",
      grcl_publisher_create(publisher_node, &publisher_create_options, &publisher),
      GRCL_OK) != 0 ||
    expect_result(
      "create subscription",
      grcl_subscription_create(
        subscription_node,
        &subscription_create_options,
        &subscription),
      GRCL_OK) != 0 ||
    expect_result(
      "create executor",
      grcl_executor_create(runtime, &executor_create_options, &executor),
      GRCL_OK) != 0 ||
    expect_result("add publisher node", grcl_executor_add_node(executor, publisher_node), GRCL_OK) != 0 ||
    expect_result(
      "add subscription node",
      grcl_executor_add_node(executor, subscription_node),
      GRCL_OK) != 0 ||
    expect_result("start runtime", grcl_runtime_start(runtime), GRCL_OK) != 0) {
    failures = 1;
    goto cleanup;
  }
  started = 1;

  if (expect_result(
      "take before spin",
      grcl_subscription_take_bytes(
        subscription,
        received,
        sizeof(received),
        &received_size),
      GRCL_ERROR_NO_DATA) != 0 ||
    expect_result(
      "publish payload",
      grcl_publisher_publish_bytes(publisher, payload, sizeof(payload)),
      GRCL_OK) != 0 ||
    expect_result("spin once", grcl_executor_spin_once(executor, 0u), GRCL_OK) != 0 ||
    expect_result(
      "take payload",
      grcl_subscription_take_bytes(
        subscription,
        received,
        sizeof(received),
        &received_size),
      GRCL_OK) != 0 ||
    expect_size("payload size", received_size, sizeof(payload)) != 0 ||
    expect_bytes("payload bytes", received, payload, sizeof(payload)) != 0) {
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
