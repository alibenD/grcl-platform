#include <cstddef>
#include <cstdio>
#include <cstring>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/types.h>
#include <grcl/c/version.h>

#include <grcl/cpp/executor.hpp>
#include <grcl/cpp/node.hpp>
#include <grcl/cpp/publisher.hpp>
#include <grcl/cpp/runtime.hpp>
#include <grcl/cpp/subscription.hpp>

namespace {

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xB001u,
  "cpp/example/message",
  0u,
  1u
};

static bool expect_result(const char * label, grcl_result_t actual, grcl_result_t expected)
{
  if (actual == expected) {
    return true;
  }

  std::fprintf(stderr, "FAIL %s: result %u expected %u\n", label, actual, expected);
  return false;
}

static bool expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual == expected) {
    return true;
  }

  std::fprintf(stderr, "FAIL %s: size %zu expected %zu\n", label, actual, expected);
  return false;
}

static bool expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (std::memcmp(actual, expected, size) == 0) {
    return true;
  }

  std::fprintf(stderr, "FAIL %s: payload mismatch\n", label);
  return false;
}

static grcl_runtime_options_t runtime_options()
{
  grcl_runtime_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-pub-sub-example";
  options.profile_name = "native-inprocess";
  return options;
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/examples";
  return options;
}

static grcl_publisher_options_t publisher_options()
{
  grcl_publisher_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "cpp/examples/topic.alpha";
  options.type_support = &k_message_type;
  return options;
}

static grcl_subscription_options_t subscription_options()
{
  grcl_subscription_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "cpp/examples/topic.alpha";
  options.type_support = &k_message_type;
  return options;
}

static grcl_executor_options_t executor_options()
{
  grcl_executor_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

}  // namespace

int main()
{
  grcl::Runtime runtime;
  grcl::Node talker;
  grcl::Node listener;
  grcl::Publisher publisher;
  grcl::Subscription subscription;
  grcl::Executor executor;

  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t talker_options = node_options("talker");
  grcl_node_options_t listener_options = node_options("listener");
  grcl_publisher_options_t publisher_create_options = publisher_options();
  grcl_subscription_options_t subscription_create_options = subscription_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char payload[] = {0x11u, 0x22u, 0x33u, 0x44u};
  unsigned char received[16u] = {0};
  size_t received_size = 0u;

  if (!expect_result(
        "create runtime",
        grcl::Runtime::create(&runtime, &runtime_create_options),
        GRCL_OK) ||
    !expect_result("create talker", grcl::Node::create(&talker, &runtime, &talker_options), GRCL_OK) ||
    !expect_result("create listener", grcl::Node::create(&listener, &runtime, &listener_options), GRCL_OK) ||
    !expect_result(
      "create publisher",
      grcl::Publisher::create(&publisher, &talker, &publisher_create_options),
      GRCL_OK) ||
    !expect_result(
      "create subscription",
      grcl::Subscription::create(&subscription, &listener, &subscription_create_options),
      GRCL_OK) ||
    !expect_result(
      "create executor",
      grcl::Executor::create(&executor, &runtime, &executor_create_options),
      GRCL_OK) ||
    !expect_result("add talker", executor.add_node(&talker), GRCL_OK) ||
    !expect_result("add listener", executor.add_node(&listener), GRCL_OK) ||
    !expect_result("start runtime", runtime.start(), GRCL_OK) ||
    !expect_result(
      "take before spin",
      subscription.take_bytes(received, sizeof(received), &received_size),
      GRCL_ERROR_NO_DATA) ||
    !expect_result(
      "publish bytes",
      publisher.publish_bytes(payload, sizeof(payload)),
      GRCL_OK) ||
    !expect_result("spin once", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take payload",
      subscription.take_bytes(received, sizeof(received), &received_size),
      GRCL_OK) ||
    !expect_size("received size", received_size, sizeof(payload)) ||
    !expect_bytes("received bytes", received, payload, sizeof(payload)) ||
    !expect_result("stop runtime", runtime.stop(), GRCL_OK) ||
    !expect_result("destroy executor", executor.destroy(), GRCL_OK) ||
    !expect_result("destroy subscription", subscription.destroy(), GRCL_OK) ||
    !expect_result("destroy publisher", publisher.destroy(), GRCL_OK) ||
    !expect_result("destroy listener", listener.destroy(), GRCL_OK) ||
    !expect_result("destroy talker", talker.destroy(), GRCL_OK) ||
    !expect_result("destroy runtime", runtime.destroy(), GRCL_OK)) {
    return 1;
  }

  return 0;
}
