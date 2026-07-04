"""Private boundary that mirrors the minimal `grcl-c` runtime calls."""

from __future__ import annotations

from typing import Protocol

from .result import Result


class NativeBoundary(Protocol):
    def grcl_runtime_create(self, options: object | None = None) -> tuple[int | Result, object | None]:
        ...

    def grcl_runtime_init_with_storage(
        self,
        options: object | None = None,
        storage: object | None = None,
    ) -> tuple[int | Result, object | None]:
        ...

    def grcl_runtime_start(self, runtime_handle: object) -> int | Result:
        ...

    def grcl_runtime_stop(self, runtime_handle: object) -> int | Result:
        ...

    def grcl_runtime_destroy(self, runtime_handle: object) -> int | Result:
        ...


def load_native_boundary() -> NativeBoundary:
    """Return the deferred native loader placeholder.

    Real shared-library loading is intentionally out of scope for this skeleton.
    """

    raise RuntimeError("native shared-library loading is deferred for the G6-D skeleton")


__all__ = ["NativeBoundary", "load_native_boundary"]
