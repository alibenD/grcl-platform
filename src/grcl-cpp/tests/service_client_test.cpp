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

static void expect_nonzero_request_id(const char * label, ::grcl_request_id_t request_id)
{
  if (request_id == 0u) {
    report_failure(label, "request id was zero");
  }
}

static void expect_request_id(
  const char * label,
  ::grcl_request_id_t actual,
  ::grcl_request_id_t expected)
{
  if (actual != expected) {
    report_failure(label, "request id mismatch");
  }
}

static const ::grcl_type_support_t k_request_type = {
  sizeof(::grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC001u,
  "cpp/test/request",
  0u,
  1u
};

static const ::grcl_type_support_t k_response_type = {
  sizeof(::grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC002u,
  "cpp/test/response",
  0u,
  1u
};

static const ::grcl_type_support_t k_other_request_type = {
  sizeof(::grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0xC003u,
  "cpp/test/request-other",
  0u,
  1u
};

static void init_runtime_options(::grcl_runtime_options_t & options)
{
  std::memset(&options, 0, sizeof(options));
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.runtime_name = "grcl-cpp-service-client";
  options.profile_name = "native-inprocess";
}

static ::grcl_node_options_t node_options(const char * name)
{
  ::grcl_node_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.node_name = name;
  options.node_namespace = "/cpp";
  return options;
}

static ::grcl_service_options_t service_options(
  const char * service_name,
  const ::grcl_type_support_t * request_type,
  const ::grcl_type_support_t * response_type)
{
  ::grcl_service_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
  options.request_type_support = request_type;
  options.response_type_support = response_type;
  return options;
}

static ::grcl_client_options_t client_options(
  const char * service_name,
  const ::grcl_type_support_t * request_type,
  const ::grcl_type_support_t * response_type)
{
  ::grcl_client_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
  options.request_type_support = request_type;
  options.response_type_support = response_type;
  return options;
}

static ::grcl_executor_options_t executor_options()
{
  ::grcl_executor_options_t options{};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static int test_request_reply_delivery_and_request_id_forwarding()
{
  ::grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  grcl::Node server_node;
  grcl::Node client_node;
  grcl::Service service;
  grcl::Client client;
  grcl::Executor executor;
  const unsigned char request[] = {0x10u, 0x20u, 0x30u};
  const unsigned char response[] = {0xA1u, 0xB2u, 0xC3u, 0xD4u};
  unsigned char received[16u] = {0};
  size_t received_size = 0u;
  ::grcl_request_id_t request_id = 0u;
  ::grcl_request_id_t taken_id = 0u;

  init_runtime_options(runtime_create_options);
  ::grcl_node_options_t server_node_options = node_options("server");
  ::grcl_node_options_t client_node_options = node_options("client");
  ::grcl_service_options_t service_create_options =
    service_options("cpp/service.alpha", &k_request_type, &k_response_type);
  ::grcl_client_options_t client_create_options =
    client_options("cpp/service.alpha", &k_request_type, &k_response_type);
  ::grcl_executor_options_t executor_create_options = executor_options();

  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  expect_result("server node create", grcl::Node::create(&server_node, &runtime, &server_node_options), GRCL_OK);
  expect_result("client node create", grcl::Node::create(&client_node, &runtime, &client_node_options), GRCL_OK);
  expect_result("service create", grcl::Service::create(&service, &server_node, &service_create_options), GRCL_OK);
  expect_result("client create", grcl::Client::create(&client, &client_node, &client_create_options), GRCL_OK);
  expect_result("executor create", grcl::Executor::create(&executor, &runtime, &executor_create_options), GRCL_OK);
  expect_result("executor add server", executor.add_node(&server_node), GRCL_OK);
  expect_result("executor add client", executor.add_node(&client_node), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);

  expect_result(
    "service no request before send",
    service.take_request_bytes(received, sizeof(received), &received_size, &taken_id),
    GRCL_ERROR_NO_DATA);
  expect_result(
    "send request",
    client.send_request_bytes(request, sizeof(request), &request_id),
    GRCL_OK);
  expect_nonzero_request_id("request id nonzero", request_id);
  expect_result(
    "client no response before spin",
    client.take_response_bytes(request_id, received, sizeof(received), &received_size),
    GRCL_ERROR_NO_DATA);
  expect_result("spin request delivery", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "take request",
    service.take_request_bytes(received, sizeof(received), &received_size, &taken_id),
    GRCL_OK);
  expect_request_id("request id forwarded", taken_id, request_id);
  expect_size("request size", received_size, sizeof(request));
  expect_bytes("request payload", received, request, sizeof(request));

  std::memset(received, 0, sizeof(received));
  expect_result(
    "send response",
    service.send_response_bytes(taken_id, response, sizeof(response)),
    GRCL_OK);
  expect_result("spin response delivery", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "take response",
    client.take_response_bytes(request_id, received, sizeof(received), &received_size),
    GRCL_OK);
  expect_size("response size", received_size, sizeof(response));
  expect_bytes("response payload", received, response, sizeof(response));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("destroy executor", executor.destroy(), GRCL_OK);
  expect_result("destroy client", client.destroy(), GRCL_OK);
  expect_result("destroy service", service.destroy(), GRCL_OK);
  expect_result("destroy client node", client_node.destroy(), GRCL_OK);
  expect_result("destroy server node", server_node.destroy(), GRCL_OK);
  expect_result("destroy runtime", runtime.destroy(), GRCL_OK);
  return failures;
}

static int test_negative_results_forward_without_new_semantics()
{
  ::grcl_runtime_options_t runtime_create_options{};
  grcl::Runtime runtime;
  grcl::Node server_node;
  grcl::Node client_node;
  grcl::Service service;
  grcl::Client client;
  grcl::Client missing_client;
  grcl::Client mismatched_client;
  grcl::Executor executor;
  const unsigned char request[] = {0x55u, 0x66u, 0x77u};
  const unsigned char response[] = {0x81u, 0x82u, 0x83u, 0x84u};
  unsigned char small_buffer[2u] = {0};
  unsigned char request_buffer[16u] = {0};
  unsigned char response_buffer[16u] = {0};
  size_t received_size = 0u;
  ::grcl_request_id_t request_id = 0u;
  ::grcl_request_id_t taken_id = 0u;

  init_runtime_options(runtime_create_options);
  ::grcl_node_options_t server_node_options = node_options("server-negative");
  ::grcl_node_options_t client_node_options = node_options("client-negative");
  ::grcl_service_options_t service_create_options =
    service_options("cpp/service.beta", &k_request_type, &k_response_type);
  ::grcl_client_options_t client_create_options =
    client_options("cpp/service.beta", &k_request_type, &k_response_type);
  ::grcl_client_options_t missing_client_create_options =
    client_options("cpp/service.missing", &k_request_type, &k_response_type);
  ::grcl_client_options_t mismatched_client_create_options =
    client_options("cpp/service.beta", &k_other_request_type, &k_response_type);
  ::grcl_executor_options_t executor_create_options = executor_options();

  expect_result("runtime create", grcl::Runtime::create(&runtime, &runtime_create_options), GRCL_OK);
  expect_result("server node create", grcl::Node::create(&server_node, &runtime, &server_node_options), GRCL_OK);
  expect_result("client node create", grcl::Node::create(&client_node, &runtime, &client_node_options), GRCL_OK);
  expect_result("service create", grcl::Service::create(&service, &server_node, &service_create_options), GRCL_OK);
  expect_result("client create", grcl::Client::create(&client, &client_node, &client_create_options), GRCL_OK);
  expect_result(
    "missing client create",
    grcl::Client::create(&missing_client, &client_node, &missing_client_create_options),
    GRCL_OK);
  expect_result(
    "mismatched client create",
    grcl::Client::create(&mismatched_client, &client_node, &mismatched_client_create_options),
    GRCL_OK);
  expect_result("executor create", grcl::Executor::create(&executor, &runtime, &executor_create_options), GRCL_OK);
  expect_result("executor add server", executor.add_node(&server_node), GRCL_OK);
  expect_result("executor add client", executor.add_node(&client_node), GRCL_OK);
  expect_result("runtime start", runtime.start(), GRCL_OK);

  expect_result(
    "unknown response id not found",
    client.take_response_bytes(999u, small_buffer, sizeof(small_buffer), &received_size),
    GRCL_ERROR_NOT_FOUND);
  expect_result(
    "missing service",
    missing_client.send_request_bytes(request, sizeof(request), &request_id),
    GRCL_ERROR_PEER_UNAVAILABLE);
  expect_result(
    "mismatched request type",
    mismatched_client.send_request_bytes(request, sizeof(request), &request_id),
    GRCL_ERROR_TYPE_MISMATCH);
  expect_result("send request", client.send_request_bytes(request, sizeof(request), &request_id), GRCL_OK);
  expect_result("spin request delivery", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "small request buffer",
    service.take_request_bytes(small_buffer, sizeof(small_buffer), &received_size, &taken_id),
    GRCL_ERROR_CAPACITY_EXCEEDED);
  expect_result(
    "retake request",
    service.take_request_bytes(request_buffer, sizeof(request_buffer), &received_size, &taken_id),
    GRCL_OK);
  expect_result(
    "send response",
    service.send_response_bytes(taken_id, response, sizeof(response)),
    GRCL_OK);
  expect_result("spin response delivery", executor.spin_once(0u), GRCL_OK);
  expect_result(
    "small response buffer",
    client.take_response_bytes(request_id, small_buffer, sizeof(small_buffer), &received_size),
    GRCL_ERROR_CAPACITY_EXCEEDED);

  expect_result(
    "retake response",
    client.take_response_bytes(
      request_id,
      response_buffer,
      sizeof(response_buffer),
      &received_size),
    GRCL_OK);
  expect_bytes("response bytes after capacity retry", response_buffer, response, sizeof(response));

  expect_result("runtime stop", runtime.stop(), GRCL_OK);
  expect_result("destroy executor", executor.destroy(), GRCL_OK);
  expect_result("destroy mismatched client", mismatched_client.destroy(), GRCL_OK);
  expect_result("destroy missing client", missing_client.destroy(), GRCL_OK);
  expect_result("destroy client", client.destroy(), GRCL_OK);
  expect_result("destroy service", service.destroy(), GRCL_OK);
  expect_result("destroy client node", client_node.destroy(), GRCL_OK);
  expect_result("destroy server node", server_node.destroy(), GRCL_OK);
  expect_result("destroy runtime", runtime.destroy(), GRCL_OK);
  return failures;
}

}  // namespace

int main()
{
  test_request_reply_delivery_and_request_id_forwarding();
  test_negative_results_forward_without_new_semantics();

  if (failures != 0) {
    std::fprintf(stderr, "service_client_test: %d failure(s)\n", failures);
    return 1;
  }

  std::puts("service_client_test: ok");
  return 0;
}
