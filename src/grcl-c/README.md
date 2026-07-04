# grcl-c

`grcl-c` is the core C ABI and semantic contract for GRCL Platform.

Initial scope:

- runtime lifecycle
- node and endpoint ownership
- controlled storage
- result and error model
- capability query and negotiation
- backend SPI host boundary

M1 closes the first minimal runnable runtime path in `src/runtime.c` with a private default
null/native-test backend descriptor and a narrow public runtime diagnostics accessor. The core
runtime keeps ownership of public runtime handles, stores the selected backend descriptor and
backend-private state pointer internally, calls backend create/start/stop/destroy hooks during valid
lifecycle transitions, routes capability query and negotiation calls through the backend SPI, and
delegates diagnostics retrieval through the backend SPI before falling back to the latest core
lifecycle diagnostic.

The M1 capability path is intentionally deterministic and narrow. The default backend reports a
null/native-test runtime identity, runtime-only graph projection support, static-storage and
bounded-capacity summary flags, deterministic destroy support, zero transport/QoS/encoding/security
descriptor counts, basic diagnostics support, and simple accepted/degraded/rejected negotiation over
the fixed fields already declared in `capability.h`.

Publish/subscribe, service/client, executor scheduling, transport, ROS2, SDK wrappers, management,
auth, remote/event stream behavior, descriptor iterator APIs, and public backend registration
remain outside this module's current implementation scope.
