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
#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>
#include <grcl/cpp/service.hpp>

namespace {

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xB101u,
  "cpp/example/request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xB102u,
  "cpp/example/response",
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
    (unsigned long long)actual,
    (unsigned long long)expected);
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
  options.runtime_name = "grcl-cpp-service-client-example";
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

static grcl_service_options_t service_options()
{
  grcl_service_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "cpp/examples/service.alpha";
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_client_options_t client_options()
{
  grcl_client_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = "cpp/examples/service.alpha";
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

}  // namespace

int main()
{
  grcl::Runtime runtime;
  grcl::Node service_node;
  grcl::Node client_node;
  grcl::Service service;
  grcl::Client client;
  grcl::Executor executor;

  grcl_runtime_options_t runtime_create_options = runtime_options();
  grcl_node_options_t service_node_create_options = node_options("server");
  grcl_node_options_t client_node_create_options = node_options("client");
  grcl_service_options_t service_create_options = service_options();
  grcl_client_options_t client_create_options = client_options();
  grcl_executor_options_t executor_create_options = executor_options();
  const unsigned char request[] = {0x11u, 0x22u, 0x33u};
  const unsigned char response[] = {0x91u, 0x92u, 0x93u, 0x94u};
  unsigned char received[32u] = {0};
  size_t received_size = 0u;
  grcl_request_id_t sent_id = 0u;
  grcl_request_id_t taken_id = 0u;

  if (!expect_result(
        "create runtime",
        grcl::Runtime::create(&runtime, &runtime_create_options),
        GRCL_OK) ||
    !expect_result(
      "create service node",
      grcl::Node::create(&service_node, &runtime, &service_node_create_options),
      GRCL_OK) ||
    !expect_result(
      "create client node",
      grcl::Node::create(&client_node, &runtime, &client_node_create_options),
      GRCL_OK) ||
    !expect_result(
      "create service",
      grcl::Service::create(&service, &service_node, &service_create_options),
      GRCL_OK) ||
    !expect_result(
      "create client",
      grcl::Client::create(&client, &client_node, &client_create_options),
      GRCL_OK) ||
    !expect_result(
      "create executor",
      grcl::Executor::create(&executor, &runtime, &executor_create_options),
      GRCL_OK) ||
    !expect_result("add service node", executor.add_node(&service_node), GRCL_OK) ||
    !expect_result("add client node", executor.add_node(&client_node), GRCL_OK) ||
    !expect_result("start runtime", runtime.start(), GRCL_OK) ||
    !expect_result(
      "take before request dispatch",
      service.take_request_bytes(received, sizeof(received), &received_size, &taken_id),
      GRCL_ERROR_NO_DATA) ||
    !expect_result(
      "send request",
      client.send_request_bytes(request, sizeof(request), &sent_id),
      GRCL_OK) ||
    !expect_result(
      "take response before request spin",
      client.take_response_bytes(sent_id, received, sizeof(received), &received_size),
      GRCL_ERROR_NO_DATA) ||
    !expect_result("spin request", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take request",
      service.take_request_bytes(received, sizeof(received), &received_size, &taken_id),
      GRCL_OK) ||
    !expect_request_id("request id preserved", taken_id, sent_id) ||
    !expect_size("request size", received_size, sizeof(request)) ||
    !expect_bytes("request bytes", received, request, sizeof(request)) ||
    !expect_result(
      "send response",
      service.send_response_bytes(taken_id, response, sizeof(response)),
      GRCL_OK)) {
    return 1;
  }

  std::memset(received, 0, sizeof(received));

  if (!expect_result(
        "take response before response spin",
        client.take_response_bytes(sent_id, received, sizeof(received), &received_size),
        GRCL_ERROR_NO_DATA) ||
    !expect_result("spin response", executor.spin_once(0u), GRCL_OK) ||
    !expect_result(
      "take response",
      client.take_response_bytes(sent_id, received, sizeof(received), &received_size),
      GRCL_OK) ||
    !expect_size("response size", received_size, sizeof(response)) ||
    !expect_bytes("response bytes", received, response, sizeof(response)) ||
    !expect_result("stop runtime", runtime.stop(), GRCL_OK) ||
    !expect_result("destroy executor", executor.destroy(), GRCL_OK) ||
    !expect_result("destroy client", client.destroy(), GRCL_OK) ||
    !expect_result("destroy service", service.destroy(), GRCL_OK) ||
    !expect_result("destroy client node", client_node.destroy(), GRCL_OK) ||
    !expect_result("destroy service node", service_node.destroy(), GRCL_OK) ||
    !expect_result("destroy runtime", runtime.destroy(), GRCL_OK)) {
    return 1;
  }

  return 0;
}
