"""Runtime ownership wrapper over the private native boundary."""

from __future__ import annotations

from ._native import NativeBoundary, load_native_boundary
from .result import Result, from_native_result


class Runtime:
    """Owns exactly one native runtime handle."""

    __slots__ = ("_native", "_handle", "_closed")

    def __init__(self, native: NativeBoundary, handle: object) -> None:
        self._native = native
        self._handle = handle
        self._closed = False

    @classmethod
    def create(
        cls,
        native: NativeBoundary | None = None,
        options: object | None = None,
    ) -> "Runtime":
        boundary = native if native is not None else load_native_boundary()
        status, handle = boundary.grcl_runtime_create(options)
        result = from_native_result(status)
        if result is not Result.OK or handle is None:
            raise RuntimeError(f"grcl_runtime_create failed with {result.name}")
        return cls(boundary, handle)

    @classmethod
    def init_with_storage(
        cls,
        native: NativeBoundary | None = None,
        options: object | None = None,
        storage: object | None = None,
    ) -> "Runtime":
        boundary = native if native is not None else load_native_boundary()
        status, handle = boundary.grcl_runtime_init_with_storage(options, storage)
        result = from_native_result(status)
        if result is not Result.OK or handle is None:
            raise RuntimeError(f"grcl_runtime_init_with_storage failed with {result.name}")
        return cls(boundary, handle)

    @property
    def closed(self) -> bool:
        return self._closed

    def start(self) -> Result:
        if self._closed:
            return Result.ERROR_BAD_STATE
        return from_native_result(self._native.grcl_runtime_start(self._handle))

    def stop(self) -> Result:
        if self._closed:
            return Result.ERROR_BAD_STATE
        return from_native_result(self._native.grcl_runtime_stop(self._handle))

    def close(self) -> Result:
        if self._closed:
            return Result.OK
        handle = self._handle
        self._handle = None
        self._closed = True
        return from_native_result(self._native.grcl_runtime_destroy(handle))

    def __enter__(self) -> "Runtime":
        return self

    def __exit__(self, exc_type, exc, tb) -> bool:
        self.close()
        return False


__all__ = ["Runtime"]
