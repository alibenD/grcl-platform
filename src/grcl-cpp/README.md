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
- start/stop/destroy and node/executor forwarding without introducing new runtime semantics
- local smoke tests and compile scripts that build directly against the existing `grcl-c`
  implementation

Service/client, params, and broader runnable C++ examples remain later M5 batches.
