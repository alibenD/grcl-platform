#include <stddef.h>
#include <stdalign.h>

#include <grcl/c/result.h>
#include <grcl/c/runtime.h>
#include <grcl/c/storage.h>
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

static int expect_non_null(const char * label, const void * value)
{
  if (value != NULL) {
    return 0;
  }

  (void)label;
  return 1;
}

static int expect_pointer(
  const char * label,
  const void * actual,
  const void * expected)
{
  if (actual == expected) {
    return 0;
  }

  (void)label;
  return 1;
}

typedef struct runtime_storage_fixture {
  union {
    max_align_t alignment;
    unsigned char bytes[1024u];
  } runtime_object;
  grcl_storage_region_t runtime_region;
  grcl_storage_t storage;
} runtime_storage_fixture_t;

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x101u,
  "test/message",
  0u,
  1u
};

static const grcl_type_support_t k_request_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x201u,
  "test/request",
  0u,
  1u
};

static const grcl_type_support_t k_response_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x202u,
  "test/response",
  0u,
  1u
};

static grcl_storage_t bounded_storage_template(void)
{
  grcl_storage_t storage;
  storage.struct_size = sizeof(storage);
  storage.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  storage.flags = GRCL_STORAGE_FLAG_BOUNDED_CAPACITIES;
  storage.regions = NULL;
  storage.region_count = 0u;
  storage.runtime_object_bytes = 0u;
  storage.node_table_capacity = 0u;
  storage.endpoint_table_capacity = 0u;
  storage.session_table_capacity = 0u;
  storage.channel_table_capacity = 0u;
  storage.message_buffer_bytes = 0u;
  storage.graph_cache_bytes = 0u;
  storage.diagnostics_buffer_bytes = 0u;
  storage.executor_state_bytes = 0u;
  return storage;
}

static void init_runtime_storage_fixture(
  runtime_storage_fixture_t * fixture,
  size_t node_capacity,
  size_t endpoint_capacity,
  size_t executor_state_bytes)
{
  fixture->runtime_region.struct_size = sizeof(fixture->runtime_region);
  fixture->runtime_region.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  fixture->runtime_region.kind = GRCL_STORAGE_REGION_KIND_RUNTIME_OBJECT;
  fixture->runtime_region.data = fixture->runtime_object.bytes;
  fixture->runtime_region.bytes = sizeof(fixture->runtime_object.bytes);
  fixture->runtime_region.alignment = alignof(max_align_t);
  fixture->runtime_region.flags = GRCL_STORAGE_FLAG_NONE;

  fixture->storage = bounded_storage_template();
  fixture->storage.regions = &fixture->runtime_region;
  fixture->storage.region_count = 1u;
  fixture->storage.runtime_object_bytes = sizeof(fixture->runtime_object.bytes);
  fixture->storage.node_table_capacity = node_capacity;
  fixture->storage.endpoint_table_capacity = endpoint_capacity;
  fixture->storage.executor_state_bytes = executor_state_bytes;
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

static grcl_publisher_options_t publisher_options(const char * topic_name)
{
  grcl_publisher_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = topic_name;
  options.type_support = &k_message_type;
  return options;
}

static grcl_subscription_options_t subscription_options(const char * topic_name)
{
  grcl_subscription_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.topic_name = topic_name;
  options.type_support = &k_message_type;
  return options;
}

static grcl_service_options_t service_options(const char * service_name)
{
  grcl_service_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
  options.request_type_support = &k_request_type;
  options.response_type_support = &k_response_type;
  return options;
}

static grcl_client_options_t client_options(const char * service_name)
{
  grcl_client_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.service_name = service_name;
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

static int test_node_create_get_runtime_destroy_happy_path(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_runtime_t * node_runtime = NULL;
  grcl_node_options_t options = node_options("alpha");

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "create node",
      grcl_node_create(runtime, &options, &node),
      GRCL_OK) != 0 ||
    expect_non_null("node handle", node) != 0 ||
    expect_result(
      "node get runtime",
      grcl_node_get_runtime(node, &node_runtime),
      GRCL_OK) != 0 ||
    expect_pointer("node runtime", node_runtime, runtime) != 0 ||
    expect_result("destroy node", grcl_node_destroy(node), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_runtime_destroy_cleans_up_undisposed_node(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_node_options_t options = node_options("alpha");

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &options, &node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime with node", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_endpoint_family_create_get_endpoint_destroy_happy_paths(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_endpoint_t * endpoint = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options = publisher_options("topic.a");
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.a");
  grcl_service_options_t service_create_options = service_options("service.a");
  grcl_client_options_t client_create_options = client_options("service.a");

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "create publisher",
      grcl_publisher_create(node, &publisher_create_options, &publisher),
      GRCL_OK) != 0 ||
    expect_result(
      "publisher get endpoint",
      grcl_publisher_get_endpoint(publisher, &endpoint),
      GRCL_OK) != 0 ||
    expect_non_null("publisher endpoint", endpoint) != 0 ||
    expect_result("destroy publisher", grcl_publisher_destroy(publisher), GRCL_OK) != 0 ||
    expect_result(
      "create subscription",
      grcl_subscription_create(
        node,
        &subscription_create_options,
        &subscription),
      GRCL_OK) != 0 ||
    expect_result(
      "subscription get endpoint",
      grcl_subscription_get_endpoint(subscription, &endpoint),
      GRCL_OK) != 0 ||
    expect_non_null("subscription endpoint", endpoint) != 0 ||
    expect_result("destroy subscription", grcl_subscription_destroy(subscription), GRCL_OK) != 0 ||
    expect_result(
      "create service",
      grcl_service_create(node, &service_create_options, &service),
      GRCL_OK) != 0 ||
    expect_result(
      "service get endpoint",
      grcl_service_get_endpoint(service, &endpoint),
      GRCL_OK) != 0 ||
    expect_non_null("service endpoint", endpoint) != 0 ||
    expect_result("destroy service", grcl_service_destroy(service), GRCL_OK) != 0 ||
    expect_result(
      "create client",
      grcl_client_create(node, &client_create_options, &client),
      GRCL_OK) != 0 ||
    expect_result(
      "client get endpoint",
      grcl_client_get_endpoint(client, &endpoint),
      GRCL_OK) != 0 ||
    expect_non_null("client endpoint", endpoint) != 0 ||
    expect_result("destroy client", grcl_client_destroy(client), GRCL_OK) != 0 ||
    expect_result("destroy node", grcl_node_destroy(node), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_node_destroy_cleans_owned_endpoint_family_objects(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options = publisher_options("topic.a");
  grcl_subscription_options_t subscription_create_options =
    subscription_options("topic.a");
  grcl_service_options_t service_create_options = service_options("service.a");
  grcl_client_options_t client_create_options = client_options("service.a");

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK ||
    grcl_subscription_create(
      node,
      &subscription_create_options,
      &subscription) != GRCL_OK ||
    grcl_service_create(node, &service_create_options, &service) != GRCL_OK ||
    grcl_client_create(node, &client_create_options, &client) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result("destroy node", grcl_node_destroy(node), GRCL_OK) != 0 ||
    expect_result(
      "publisher re-destroy",
      grcl_publisher_destroy(publisher),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "subscription re-destroy",
      grcl_subscription_destroy(subscription),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "service re-destroy",
      grcl_service_destroy(service),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "client re-destroy",
      grcl_client_destroy(client),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_executor_membership_and_spin_semantics(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_executor_options_t create_executor_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK ||
    grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "create executor",
      grcl_executor_create(runtime, &create_executor_options, &executor),
      GRCL_OK) != 0 ||
    expect_non_null("executor handle", executor) != 0 ||
    expect_result("executor add node", grcl_executor_add_node(executor, node), GRCL_OK) != 0 ||
    expect_result("executor remove node", grcl_executor_remove_node(executor, node), GRCL_OK) != 0 ||
    expect_result(
      "executor spin once",
      grcl_executor_spin_once(executor, 0u),
      GRCL_ERROR_UNSUPPORTED_CAPABILITY) != 0 ||
    expect_result("destroy executor", grcl_executor_destroy(executor), GRCL_OK) != 0 ||
    expect_result("destroy node", grcl_node_destroy(node), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_invalid_arguments_and_bad_state(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_subscription_t * subscription = NULL;
  grcl_service_t * service = NULL;
  grcl_client_t * client = NULL;
  grcl_executor_t * executor = NULL;
  grcl_endpoint_t * endpoint = NULL;
  grcl_node_options_t empty_node = node_options("");
  grcl_node_options_t initial_node = node_options("alpha");
  grcl_node_options_t started_node = node_options("beta");
  grcl_publisher_options_t missing_topic = publisher_options("topic.a");
  grcl_publisher_options_t missing_type = publisher_options("topic.a");
  grcl_subscription_options_t missing_sub_type = subscription_options("topic.a");
  grcl_service_options_t missing_service_name = service_options("service.a");
  grcl_client_options_t missing_client_response = client_options("service.a");
  grcl_executor_options_t create_executor_options = executor_options();

  missing_topic.topic_name = "";
  missing_type.type_support = NULL;
  missing_sub_type.type_support = NULL;
  missing_service_name.service_name = "";
  missing_client_response.response_type_support = NULL;

  if (expect_result("null node create runtime", grcl_node_create(NULL, NULL, NULL), GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "null node get runtime",
      grcl_node_get_runtime(NULL, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "null publisher endpoint output",
      grcl_publisher_get_endpoint(NULL, &endpoint),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "null executor add node",
      grcl_executor_add_node(NULL, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    return 1;
  }

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &initial_node, &node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "empty node name",
      grcl_node_create(runtime, &empty_node, &node),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "empty publisher topic",
      grcl_publisher_create(node, &missing_topic, &publisher),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "missing publisher type support",
      grcl_publisher_create(node, &missing_type, &publisher),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "missing subscription type support",
      grcl_subscription_create(node, &missing_sub_type, &subscription),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "empty service name",
      grcl_service_create(node, &missing_service_name, &service),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "missing client response type",
      grcl_client_create(node, &missing_client_response, &client),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "null executor output",
      grcl_executor_create(runtime, &create_executor_options, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_node_destroy(node);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_node_destroy(node);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "create node while started",
      grcl_node_create(runtime, &started_node, &node),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result("destroy node while started", grcl_node_destroy(node), GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "create executor while started",
      grcl_executor_create(runtime, &create_executor_options, &executor),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_node_destroy(node);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (grcl_runtime_stop(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result("destroy node after stop", grcl_node_destroy(node), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_capacity_exhaustion_uses_bounded_storage(void)
{
  runtime_storage_fixture_t fixture;
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node_a = NULL;
  grcl_node_t * node_b = NULL;
  grcl_publisher_t * publisher_a = NULL;
  grcl_publisher_t * publisher_b = NULL;
  grcl_executor_t * executor_a = NULL;
  grcl_executor_t * executor_b = NULL;
  grcl_node_options_t node_alpha = node_options("alpha");
  grcl_node_options_t node_beta = node_options("beta");
  grcl_publisher_options_t publisher_topic_a = publisher_options("topic.a");
  grcl_publisher_options_t publisher_topic_b = publisher_options("topic.b");
  grcl_executor_options_t create_executor_options = executor_options();

  init_runtime_storage_fixture(&fixture, 1u, 1u, 1u);
  if (grcl_runtime_init_with_storage(NULL, &fixture.storage, &runtime) != GRCL_OK) {
    return 1;
  }

  if (expect_result(
      "create first node",
      grcl_node_create(runtime, &node_alpha, &node_a),
      GRCL_OK) != 0 ||
    expect_result(
      "node table capacity exceeded",
      grcl_node_create(runtime, &node_beta, &node_b),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result(
      "create first publisher",
      grcl_publisher_create(node_a, &publisher_topic_a, &publisher_a),
      GRCL_OK) != 0 ||
    expect_result(
      "endpoint table capacity exceeded",
      grcl_publisher_create(node_a, &publisher_topic_b, &publisher_b),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0 ||
    expect_result(
      "create first executor",
      grcl_executor_create(runtime, &create_executor_options, &executor_a),
      GRCL_OK) != 0 ||
    expect_result(
      "executor capacity exceeded",
      grcl_executor_create(runtime, &create_executor_options, &executor_b),
      GRCL_ERROR_CAPACITY_EXCEEDED) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

int main(void)
{
  int failures = 0;

  failures += test_node_create_get_runtime_destroy_happy_path();
  failures += test_runtime_destroy_cleans_up_undisposed_node();
  failures += test_endpoint_family_create_get_endpoint_destroy_happy_paths();
  failures += test_node_destroy_cleans_owned_endpoint_family_objects();
  failures += test_executor_membership_and_spin_semantics();
  failures += test_invalid_arguments_and_bad_state();
  failures += test_capacity_exhaustion_uses_bounded_storage();

  return failures == 0 ? 0 : 1;
}
