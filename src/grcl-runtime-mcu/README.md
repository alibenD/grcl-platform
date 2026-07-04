# grcl-runtime-mcu

`grcl-runtime-mcu` is the planned profile-limited bare-metal and RTOS runtime/backend module for
GRCL Platform.

It is responsible for declared MCU profiles, adapters, capability reporting, bounded memory
policies, and fail-fast handling for unsupported features. It must not imply full GRCL graph or
runtime support by default; only the accepted profile is in scope for any concrete backend.

No runtime behavior is implemented in this skeleton phase.
