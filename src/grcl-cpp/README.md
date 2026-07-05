# grcl-cpp

`grcl-cpp` is the C++ SDK layer over `grcl-c`.

It must not define independent runtime lifecycle, ownership, capability, or backend semantics.
Its role is to provide idiomatic C++ ownership and type-safe wrappers that preserve the `grcl-c`
contract as the semantic source of truth.

The current skeleton provides:

- `grcl::Result` as a direct alias of `grcl_result_t`
- `grcl::Runtime` as a minimal RAII wrapper over one `grcl_runtime_t *`
- runtime capability query, negotiation, and diagnostics forwarding without introducing new policy
- `grcl::Node` as a move-only ownership wrapper over one `grcl_node_t *`
- `grcl::Executor` as a move-only ownership wrapper over one `grcl_executor_t *`
- `grcl::Publisher` as a move-only ownership wrapper over one `grcl_publisher_t *`
- `grcl::Subscription` as a move-only ownership wrapper over one `grcl_subscription_t *`
- `grcl::Service` as a move-only ownership wrapper over one `grcl_service_t *`
- `grcl::Client` as a move-only ownership wrapper over one `grcl_client_t *`
- `grcl::Params` as a thin non-owning runtime-local params facade over one `grcl_runtime_t *`
- start/stop/destroy and node/executor forwarding without introducing new runtime semantics
- local smoke tests and compile scripts that build directly against the existing `grcl-c`
  implementation

`grcl::Params` preserves the raw `grcl-c` runtime-local parameter contract:

- `set` forwards `grcl_param_record_t` directly and preserves copy-in behavior
- `get` forwards caller-provided output buffers and required-size reporting
- `list` forwards newline-delimited caller-buffer output and param-count reporting

Broader combined runnable C++ examples remain later M5 batches.
