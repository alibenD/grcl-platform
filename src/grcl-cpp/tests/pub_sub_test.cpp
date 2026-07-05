#include <cstddef>
#include <cstdio>
#include <cstring>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
#include <grcl/c/types.h>
#include <grcl/c/version.h>

#include <grcl/cpp/executor.hpp>
#include <grcl/cpp/node.hpp>
#include <grcl/cpp/publisher.hpp>
#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>
#include <grcl/cpp/subscription.hpp>

namespace {

static int failures = 0;

static void report_failure(const char * label, const char * detail)
{
  std::fprintf(stderr, "%s: %s\n", label, detail);
  ++failures;
}

static void expect_result(const char * label, grcl::Result actual, grcl::Result expected)
{
  if (actual != expected) {
    std::fprintf(
      stderr,
      "%s: expected %u, got %u\n",
      label,
      static_cast<unsigned>(expected),
      static_cast<unsigned>(actual));
    ++failures;
  }
}

static void expect_size(const char * label, size_t actual, size_t expected)
{
  if (actual != expected) {
    report_failure(label, "size mismatch");
  }
}

static void expect_bytes(
  const char * label,
  const unsigned char * actual,
  const unsigned char * expected,
  size_t size)
{
  if (std::memcmp(actual, expected, size) != 0) {
    report_failure(label, "payload mismatch");
  }
}

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xA001u,
  "cpp/test/message",
  0u,
  1u
};

static const grcl_type_support_t k_other_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xA002u,
  "cpp/test/other",
  0u,
  1u
};

static void init_runtime_options(grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-pubsub";
  options.profile_name = "native-inprocess";
}

static grcl_node_options_t node_options(const char * name)
{
  grcl_node_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/cpp";
  return options;
}

static grcl_publisher_options_t publisher_options(
  const char * topic_name,
  const grcl_type_support_t * type_support)
{
  grcl_publisher_options_t options{};
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
  grcl_subscription_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = topic_name;
  options.type_support = type_support;
  return options;
}

static grcl_executor_options_t executor_options()
{
  grcl_executor_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static int test_pub_sub_delivery_and_take_semantics()
{
  grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  grcl::Node talker;
  grcl::Node listener;
  grcl::Publisher publisher;
  grcl::Subscription subscription;
  grcl::Executor executor;
  unsigned char payload[] = {0x10u, 0x20u, 0x30u, 0x40u};
  const unsigned char expected[] = {0x10u, 0x20u, 0x30u, 0x40u};
  unsigned char received[16u] = {0};
  size_t received_size = 0u;

  init_runtime_options(runtime_create_options);
  grcl_node_options_t talker_options = node_options("talker");
  grcl_node_options_t listener_options = node_options("listener");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("cpp/topic.alpha", &k_message_type);
  grcl_subscription_options_t subscription_create_options =
    subscription_options("cpp/topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  expect_result("talker create", grcl::Node::create(&talker, &runtime, &talker_options), GRCL_OK);
  expect_result("listener create", grcl::Node::create(&listener, &runtime, &listener_options), GRCL_OK);
  expect_result(
    "publisher create",
    grcl::Publisher::create(&publisher, &talker, &publisher_create_options),
    GRCL_OK);
  expect_result(
    "subscription create",
    grcl::Subscription::create(&subscription, &listener, &subscription_create_options),
    GRCL_OK);
  expect_result(
    "executor create",
    grcl::Executor::create(&executor, &runtime, &executor_create_options),
    GRCL_OK);
  expect_result("executor add talker", executor.add_node(&talker), GRCL_OK);
  expect_result("executor add listener", executor.add_node(&listener), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);

  expect_result(
    "take before spin",
    subscription.take_bytes(received, sizeof(received), &received_size),
    GRCL_ERROR_NO_DATA);
  expect_result("publish bytes", publisher.publish_bytes(payload, sizeof(payload)), GRCL_OK);
  std::memset(payload, 0xff, sizeof(payload));
  expect_result("spin once", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "take after spin",
    subscription.take_bytes(received, sizeof(received), &received_size),
    GRCL_OK);
  expect_size("received size", received_size, sizeof(expected));
  expect_bytes("received bytes", received, expected, sizeof(expected));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("executor destroy", executor.destroy(), GRCL_OK);
  expect_result("subscription destroy", subscription.destroy(), GRCL_OK);
  expect_result("publisher destroy", publisher.destroy(), GRCL_OK);
  expect_result("listener destroy", listener.destroy(), GRCL_OK);
  expect_result("talker destroy", talker.destroy(), GRCL_OK);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  return failures;
}

static int test_type_mismatch_and_small_buffer()
{
  grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  grcl::Node talker;
  grcl::Node listener_a;
  grcl::Node listener_b;
  grcl::Publisher publisher;
  grcl::Subscription matching_subscription;
  grcl::Subscription mismatched_subscription;
  grcl::Executor executor;
  const unsigned char payload[] = {0xAAu, 0xBBu, 0xCCu};
  unsigned char small_buffer[2u] = {0};
  size_t received_size = 0u;

  init_runtime_options(runtime_create_options);
  grcl_node_options_t talker_options = node_options("talker");
  grcl_node_options_t listener_a_options = node_options("listener-a");
  grcl_node_options_t listener_b_options = node_options("listener-b");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("cpp/topic.beta", &k_message_type);
  grcl_subscription_options_t matching_options =
    subscription_options("cpp/topic.beta", &k_message_type);
  grcl_subscription_options_t mismatched_options =
    subscription_options("cpp/topic.beta", &k_other_type);
  grcl_executor_options_t executor_create_options = executor_options();

  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  expect_result("talker create", grcl::Node::create(&talker, &runtime, &talker_options), GRCL_OK);
  expect_result("listener a create", grcl::Node::create(&listener_a, &runtime, &listener_a_options), GRCL_OK);
  expect_result("listener b create", grcl::Node::create(&listener_b, &runtime, &listener_b_options), GRCL_OK);
  expect_result(
    "publisher create",
    grcl::Publisher::create(&publisher, &talker, &publisher_create_options),
    GRCL_OK);
  expect_result(
    "matching subscription create",
    grcl::Subscription::create(&matching_subscription, &listener_a, &matching_options),
    GRCL_OK);
  expect_result(
    "mismatched subscription create",
    grcl::Subscription::create(&mismatched_subscription, &listener_b, &mismatched_options),
    GRCL_OK);
  expect_result(
    "executor create",
    grcl::Executor::create(&executor, &runtime, &executor_create_options),
    GRCL_OK);
  expect_result("executor add talker", executor.add_node(&talker), GRCL_OK);
  expect_result("executor add listener a", executor.add_node(&listener_a), GRCL_OK);
  expect_result("executor add listener b", executor.add_node(&listener_b), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);

  expect_result("publish bytes", publisher.publish_bytes(payload, sizeof(payload)), GRCL_OK);
  expect_result("spin once", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "small buffer capacity error",
    matching_subscription.take_bytes(small_buffer, sizeof(small_buffer), &received_size),
    GRCL_ERROR_CAPACITY_EXCEEDED);
  expect_result(
    "mismatched subscription no data",
    mismatched_subscription.take_bytes(small_buffer, sizeof(small_buffer), &received_size),
    GRCL_ERROR_NO_DATA);

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("executor destroy", executor.destroy(), GRCL_OK);
  expect_result("mismatched subscription destroy", mismatched_subscription.destroy(), GRCL_OK);
  expect_result("matching subscription destroy", matching_subscription.destroy(), GRCL_OK);
  expect_result("publisher destroy", publisher.destroy(), GRCL_OK);
  expect_result("listener b destroy", listener_b.destroy(), GRCL_OK);
  expect_result("listener a destroy", listener_a.destroy(), GRCL_OK);
  expect_result("talker destroy", talker.destroy(), GRCL_OK);
  expect_result("runtime destroy", runtime.destroy(), GRCL_OK);
  return failures;
}

}  // namespace

int main()
{
  test_pub_sub_delivery_and_take_semantics();
  test_type_mismatch_and_small_buffer();

  if (failures != 0) {
    std::fprintf(stderr, "pub_sub_test: %d failure(s)\n", failures);
    return 1;
  }

  std::puts("pub_sub_test: ok");
  return 0;
}
