#include <stddef.h>
#include <stdio.h>

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

  fprintf(
    stderr,
    "EXPECT RESULT FAILED: %s actual=%d expected=%d\n",
    label,
    (int)actual,
    (int)expected);
  return 1;
}

static grcl_runtime_options_t runtime_options(const char * profile_name)
{
  grcl_runtime_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  options.profile_name = profile_name;
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

static grcl_executor_options_t executor_options(void)
{
  grcl_executor_options_t options = {0};
  options.struct_size = sizeof(options);
  options.abi_version = GRCL_C_ABI_VERSION_CURRENT;
  return options;
}

static const grcl_type_support_t k_message_type = {
  sizeof(grcl_type_support_t),
  GRCL_C_ABI_VERSION_CURRENT,
  GRCL_TYPE_SUPPORT_FLAG_NONE,
  0x401u,
  "m4/message",
  0u,
  1u
};

static int test_runtime_destroy_null_is_invalid_argument(void)
{
  return expect_result(
    "runtime destroy null",
    grcl_runtime_destroy(NULL),
    GRCL_ERROR_INVALID_ARGUMENT);
}

static int test_started_runtime_destroy_is_bad_state_but_recoverable(void)
{
  grcl_runtime_t * runtime = NULL;

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_result(
      "destroy started runtime",
      grcl_runtime_destroy(runtime),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (expect_result("stop after failed destroy", grcl_runtime_stop(runtime), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result(
    "destroy stopped runtime after failed destroy",
    grcl_runtime_destroy(runtime),
    GRCL_OK);
}

static int test_invalid_option_struct_sizes_are_rejected(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_runtime_options_t bad_runtime_options = runtime_options(NULL);
  grcl_node_options_t bad_node_options = node_options("alpha");
  grcl_executor_options_t bad_executor_options = executor_options();

  bad_runtime_options.struct_size -= 1u;
  if (expect_result(
      "runtime create invalid options size",
      grcl_runtime_create(&bad_runtime_options, &runtime),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    return 1;
  }
  if (runtime != NULL) {
    return 1;
  }

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }

  bad_node_options.struct_size -= 1u;
  if (expect_result(
      "node create invalid options size",
      grcl_node_create(runtime, &bad_node_options, &node),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (node != NULL) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  bad_executor_options.struct_size -= 1u;
  if (expect_result(
      "executor create invalid options size",
      grcl_executor_create(runtime, &bad_executor_options, &executor),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (executor != NULL) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_null_required_output_pointers_are_rejected(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  if (expect_result(
      "runtime create null output",
      grcl_runtime_create(NULL, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    return 1;
  }

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "node create null output",
      grcl_node_create(runtime, &node_create_options, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "executor create null output",
      grcl_executor_create(runtime, &executor_create_options, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "node get runtime null output",
      grcl_node_get_runtime(node, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0 ||
    expect_result(
      "publisher get endpoint null output",
      grcl_publisher_get_endpoint(publisher, NULL),
      GRCL_ERROR_INVALID_ARGUMENT) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_started_runtime_rejects_node_and_executor_create(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_executor_options_t executor_create_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_runtime_start(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "node create on started runtime",
      grcl_node_create(runtime, &node_create_options, &node),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "executor create on started runtime",
      grcl_executor_create(runtime, &executor_create_options, &executor),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_stop(runtime);
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (grcl_runtime_stop(runtime) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_destroyed_non_runtime_object_usage_returns_bad_state(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_node_t * endpoint_node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_runtime_t * owner_runtime = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_node_options_t endpoint_node_options = node_options("beta");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_node_create(runtime, &endpoint_node_options, &endpoint_node) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_publisher_create(endpoint_node, &publisher_create_options, &publisher) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (grcl_node_destroy(node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (grcl_publisher_destroy(publisher) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }
  if (grcl_executor_destroy(executor) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result(
      "destroyed node get runtime",
      grcl_node_get_runtime(node, &owner_runtime),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "create endpoint with destroyed node",
      grcl_publisher_create(node, &publisher_create_options, &publisher),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "destroyed endpoint get endpoint",
      grcl_publisher_get_endpoint(publisher, (grcl_endpoint_t **)&owner_runtime),
      GRCL_ERROR_BAD_STATE) != 0 ||
    expect_result(
      "destroyed executor add node",
      grcl_executor_add_node(executor, endpoint_node),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_cross_runtime_executor_membership_is_rejected(void)
{
  grcl_runtime_t * runtime_a = NULL;
  grcl_runtime_t * runtime_b = NULL;
  grcl_node_t * node_b = NULL;
  grcl_executor_t * executor_a = NULL;
  grcl_node_options_t node_create_options = node_options("beta");
  grcl_executor_options_t executor_create_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime_a) != GRCL_OK ||
    grcl_runtime_create(NULL, &runtime_b) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime_a);
    (void)grcl_runtime_destroy(runtime_b);
    return 1;
  }
  if (grcl_node_create(runtime_b, &node_create_options, &node_b) != GRCL_OK ||
    grcl_executor_create(runtime_a, &executor_create_options, &executor_a) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime_a);
    (void)grcl_runtime_destroy(runtime_b);
    return 1;
  }

  if (expect_result(
      "cross-runtime executor add node",
      grcl_executor_add_node(executor_a, node_b),
      GRCL_ERROR_BAD_STATE) != 0) {
    (void)grcl_runtime_destroy(runtime_a);
    (void)grcl_runtime_destroy(runtime_b);
    return 1;
  }

  if (grcl_runtime_destroy(runtime_a) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime_b);
    return 1;
  }

  return expect_result("destroy runtime b", grcl_runtime_destroy(runtime_b), GRCL_OK);
}

static int test_non_creation_order_cleanup_is_deterministic(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_publisher_t * publisher = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_publisher_options_t publisher_create_options =
    publisher_options("topic.alpha", &k_message_type);
  grcl_executor_options_t executor_create_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK ||
    grcl_publisher_create(node, &publisher_create_options, &publisher) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  if (expect_result("destroy node first", grcl_node_destroy(node), GRCL_OK) != 0 ||
    expect_result("destroy executor after node", grcl_executor_destroy(executor), GRCL_OK) != 0) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result("destroy runtime after non-creation-order cleanup", grcl_runtime_destroy(runtime), GRCL_OK);
}

static int test_runtime_destroy_cleans_remaining_objects(void)
{
  grcl_runtime_t * runtime = NULL;
  grcl_node_t * node = NULL;
  grcl_executor_t * executor = NULL;
  grcl_node_options_t node_create_options = node_options("alpha");
  grcl_executor_options_t executor_create_options = executor_options();

  if (grcl_runtime_create(NULL, &runtime) != GRCL_OK) {
    return 1;
  }
  if (grcl_node_create(runtime, &node_create_options, &node) != GRCL_OK ||
    grcl_executor_create(runtime, &executor_create_options, &executor) != GRCL_OK ||
    grcl_executor_add_node(executor, node) != GRCL_OK) {
    (void)grcl_runtime_destroy(runtime);
    return 1;
  }

  return expect_result(
    "runtime destroy with live objects",
    grcl_runtime_destroy(runtime),
    GRCL_OK);
}

int main(void)
{
  int failures = 0;

  failures += test_runtime_destroy_null_is_invalid_argument();
  failures += test_started_runtime_destroy_is_bad_state_but_recoverable();
  failures += test_invalid_option_struct_sizes_are_rejected();
  failures += test_null_required_output_pointers_are_rejected();
  failures += test_started_runtime_rejects_node_and_executor_create();
  failures += test_destroyed_non_runtime_object_usage_returns_bad_state();
  failures += test_cross_runtime_executor_membership_is_rejected();
  failures += test_non_creation_order_cleanup_is_deterministic();
  failures += test_runtime_destroy_cleans_remaining_objects();

  return failures == 0 ? 0 : 1;
}
