#include <cstddef>
#include <cstdio>
#include <cstring>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/types.h>
#include <grcl/c/version.h>

#include <grcl/cpp/client.hpp>
#include <grcl/cpp/executor.hpp>
#include <grcl/cpp/node.hpp>
#include <grcl/cpp/params.hpp>
#include <grcl/cpp/publisher.hpp>
#include <grcl/cpp/runtime.hpp>
#include <grcl/cpp/service.hpp>
#include <grcl/cpp/subscription.hpp>

namespace {

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC001u,
  "cpp/example/core-message",
  0u,
  1u
};

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC002u,
  "cpp/example/core-request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC003u,
  "cpp/example/core-response",
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

static bool expect_request_id(
  const char * label,
  grcl_request_id_t actual,
  grcl_request_id_t expected)
{
  if (actual == expected) {
    return true;
  }

  std::fprintf(
    stderr,
    "FAIL %s: request id %llu expected %llu\n",
    label,
    static_cast<unsigned long long>(actual),
    static_cast<unsigned long long>(expected));
  return false;
}

static bool expect_type(
  const char * label,
  grcl_param_type_t actual,
  grcl_param_type_t expected)
{
  if (actual == expected) {
    return true;
  }

  std::fprintf(stderr, "FAIL %s: type %u expected %u\n", label, actual, expected);
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

  std::fprintf(stderr, "FAIL %s: bytes mismatch\n", label);
  return false;
}

static bool contains_name(const char * names, size_t names_size, const char * expected_name)
{
  const size_t expected_size = std::strlen(expected_name);
  size_t start = 0u;

  while (start < names_size) {
    size_t end = start;
    while (end < names_size && names[end] != '\n') {
      ++end;
    }
    if ((end - start) == expected_size &&
      std::memcmp(names + start, expected_name, expected_size) == 0) {
      return true;
    }
    start = end + 1u;
  }

  return false;
}

static grcl_runtime_options_t runtime_options()
{
  grcl_runtime_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-core-middleware-example";
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
  options.topic_name = "cpp/examples/core.topic";
  options.type_support = &k_message_type;
  return options;
}

static grcl_subscription_options_t subscription_options()
{
  grcl_subscription_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = "cpp/examples/core.topic";
  options.type_support = &k_message_type;
  return options;
}

static grcl_service_options_t service_options()
{
  grcl_service_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "cpp/examples/core.service";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_client_options_t client_options()
{
  grcl_client_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "cpp/examples/core.service";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_executor_options_t executor_options()
{
  grcl_executor_options_t options{};
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
  grcl_param_record_t record{};
  record.struct_size = sizeof(record);
  record.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  record.name = name;
  record.type = type;
  record.value = value;
  record.value_size = value_size;
  return record;
}

}  // namespace

int main()
{
  grcl::Runtime runtime;
  grcl::Node server_node;
  grcl::Node client_node;
  grcl::Publisher publisher;
  grcl::Subscription subscription;
  grcl::Service service;
  grcl::Client client;
  grcl::Executor executor;
  grcl::Params params;

  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t server_node_create_options = node_options("server");
  grcl_node_options_t client_node_create_options = node_options("client");
  grcl_publisher_options_t publisher_create_options = publisher_options();
  grcl_subscription_options_t subscription_create_options = subscription_options();
  grcl_service_options_t service_create_options = service_options();
  grcl_client_options_t client_create_options = client_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char enabled = 1u;
  grcl_param_record_t enabled_record =
    param_record("core.enabled", GRCL_PARAM_TYPE_BOOL, &enabled, sizeof(enabled));
  const unsigned char pub_payload[] = {0x01u, 0x02u, 0x03u, 0x04u};
  const unsigned char request[] = {0x90u, 0x91u};
  const unsigned char response[] = {0xA0u, 0xA1u, 0xA2u};
  unsigned char buffer[64u] = {0};
  char names_buffer[64u] = {0};
  size_t payload_size = 0u;
  size_t names_size = 0u;
  size_t param_count = 0u;
  grcl_param_record_t out_param{};
  grcl_request_id_t sent_id = 0u;
  grcl_request_id_t taken_id = 0u;

  if (!expect_result(
        "create runtime",
        grcl::Runtime::create(&runtime, &runtime_create_options),
        GRCL_OK) ||
    !expect_result(
      "create server node",
      grcl::Node::create(&server_node, &runtime, &server_node_create_options),
      GRCL_OK) ||
    !expect_result(
      "create client node",
      grcl::Node::create(&client_node, &runtime, &client_node_create_options),
      GRCL_OK) ||
    !expect_result(
      "create publisher",
      grcl::Publisher::create(&publisher, &server_node, &publisher_create_options),
      GRCL_OK) ||
    !expect_result(
      "create subscription",
      grcl::Subscription::create(&subscription, &client_node, &subscription_create_options),
      GRCL_OK) ||
    !expect_result(
      "create service",
      grcl::Service::create(&service, &server_node, &service_create_options),
      GRCL_OK) ||
    !expect_result(
      "create client",
      grcl::Client::create(&client, &client_node, &client_create_options),
      GRCL_OK) ||
    !expect_result(
      "create executor",
      grcl::Executor::create(&executor, &runtime, &executor_create_options),
      GRCL_OK)) {
    return 1;
  }

  params.reset(&runtime);

  if (!expect_result("set runtime param", params.set(&enabled_record), GRCL_OK) ||
    !expect_result("add server node", executor.add_node(&server_node), GRCL_OK) ||
    !expect_result("add client node", executor.add_node(&client_node), GRCL_OK) ||
    !expect_result("start runtime", runtime.start(), GRCL_OK) ||
    !expect_result(
      "get runtime param",
      params.get(
        "core.enabled",
        &out_param,
        buffer,
        sizeof(buffer),
        &payload_size),
      GRCL_OK) ||
    !expect_type("param type", out_param.type, GRCL_PARAM_TYPE_BOOL) ||
    !expect_size("param size", payload_size, sizeof(enabled)) ||
    !expect_bytes("param bytes", buffer, &enabled, sizeof(enabled)) ||
    !expect_result(
      "publish payload",
      publisher.publish_bytes(pub_payload, sizeof(pub_payload)),
      GRCL_OK) ||
    !expect_result("dispatch pub/sub", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take payload",
      subscription.take_bytes(buffer, sizeof(buffer), &payload_size),
      GRCL_OK) ||
    !expect_size("pub/sub payload size", payload_size, sizeof(pub_payload)) ||
    !expect_bytes("pub/sub payload", buffer, pub_payload, sizeof(pub_payload)) ||
    !expect_result(
      "send request",
      client.send_request_bytes(request, sizeof(request), &sent_id),
      GRCL_OK) ||
    !expect_result("dispatch request", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take request",
      service.take_request_bytes(buffer, sizeof(buffer), &payload_size, &taken_id),
      GRCL_OK) ||
    !expect_request_id("request id", taken_id, sent_id) ||
    !expect_size("request size", payload_size, sizeof(request)) ||
    !expect_bytes("request bytes", buffer, request, sizeof(request)) ||
    !expect_result(
      "send response",
      service.send_response_bytes(taken_id, response, sizeof(response)),
      GRCL_OK) ||
    !expect_result("dispatch response", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take response",
      client.take_response_bytes(sent_id, buffer, sizeof(buffer), &payload_size),
      GRCL_OK) ||
    !expect_size("response size", payload_size, sizeof(response)) ||
    !expect_bytes("response bytes", buffer, response, sizeof(response)) ||
    !expect_result(
      "list params",
      params.list(names_buffer, sizeof(names_buffer), &names_size, &param_count),
      GRCL_OK) ||
    !expect_size("param count", param_count, 1u) ||
    !contains_name(names_buffer, names_size, "core.enabled")) {
    std::fprintf(stderr, "FAIL combined local-core scenario\n");
    return 1;
  }

  if (!expect_result("stop runtime", runtime.stop(), GRCL_OK) ||
    !expect_result("destroy executor", executor.destroy(), GRCL_OK) ||
    !expect_result("destroy client", client.destroy(), GRCL_OK) ||
    !expect_result("destroy service", service.destroy(), GRCL_OK) ||
    !expect_result("destroy subscription", subscription.destroy(), GRCL_OK) ||
    !expect_result("destroy publisher", publisher.destroy(), GRCL_OK) ||
    !expect_result("destroy client node", client_node.destroy(), GRCL_OK) ||
    !expect_result("destroy server node", server_node.destroy(), GRCL_OK) ||
    !expect_result("destroy runtime", runtime.destroy(), GRCL_OK)) {
    return 1;
  }

  return 0;
}
