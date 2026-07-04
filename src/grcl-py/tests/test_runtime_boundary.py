from __future__ import annotations

import unittest

from grcl_py import (
    GRCL_ERROR_BAD_STATE,
    GRCL_ERROR_BACKEND_FAILURE,
    GRCL_ERROR_CAPACITY_EXCEEDED,
    GRCL_ERROR_CHANNEL_REJECTED,
    GRCL_ERROR_INCOMPATIBLE_VERSION,
    GRCL_ERROR_INTERRUPTED,
    GRCL_ERROR_INVALID_ARGUMENT,
    GRCL_ERROR_OUT_OF_MEMORY,
    GRCL_ERROR_PEER_UNAVAILABLE,
    GRCL_ERROR_TIMEOUT,
    GRCL_ERROR_UNKNOWN,
    GRCL_ERROR_UNSUPPORTED_CAPABILITY,
    GRCL_OK,
    Result,
    Runtime,
)


class FakeNativeBoundary:
    def __init__(self) -> None:
        self.runtime_handle = object()
        self.create_calls: list[object | None] = []
        self.init_calls: list[tuple[object | None, object | None]] = []
        self.start_calls: list[object] = []
        self.stop_calls: list[object] = []
        self.destroy_calls: list[object] = []

    def grcl_runtime_create(self, options: object | None = None) -> tuple[Result, object]:
        self.create_calls.append(options)
        return Result.OK, self.runtime_handle

    def grcl_runtime_init_with_storage(
        self,
        options: object | None = None,
        storage: object | None = None,
    ) -> tuple[Result, object]:
        self.init_calls.append((options, storage))
        return Result.OK, self.runtime_handle

    def grcl_runtime_start(self, runtime_handle: object) -> Result:
        self.start_calls.append(runtime_handle)
        return Result.OK

    def grcl_runtime_stop(self, runtime_handle: object) -> Result:
        self.stop_calls.append(runtime_handle)
        return Result.OK

    def grcl_runtime_destroy(self, runtime_handle: object) -> Result:
        self.destroy_calls.append(runtime_handle)
        return Result.OK


class RuntimeBoundaryTests(unittest.TestCase):
    def test_runtime_lifecycle_uses_private_boundary_and_closes_once(self) -> None:
        native = FakeNativeBoundary()

        with Runtime.create(native=native) as runtime:
            self.assertEqual(runtime.start(), Result.OK)
            self.assertEqual(runtime.stop(), Result.OK)
            self.assertFalse(runtime.closed)

        self.assertEqual(native.create_calls, [None])
        self.assertEqual(native.start_calls, [native.runtime_handle])
        self.assertEqual(native.stop_calls, [native.runtime_handle])
        self.assertEqual(native.destroy_calls, [native.runtime_handle])

        self.assertEqual(runtime.close(), Result.OK)
        self.assertEqual(native.destroy_calls, [native.runtime_handle])
        self.assertTrue(runtime.closed)

    def test_init_with_storage_uses_the_native_boundary(self) -> None:
        native = FakeNativeBoundary()
        storage = object()

        runtime = Runtime.init_with_storage(native=native, options={"name": "demo"}, storage=storage)
        self.assertEqual(native.init_calls, [({"name": "demo"}, storage)])
        self.assertEqual(runtime.close(), Result.OK)
        self.assertEqual(native.destroy_calls, [native.runtime_handle])

    def test_result_constants_match_grcl_result_values(self) -> None:
        expected_values = {
            "GRCL_OK": 0x0000,
            "GRCL_ERROR_INVALID_ARGUMENT": 0x0101,
            "GRCL_ERROR_BAD_STATE": 0x0102,
            "GRCL_ERROR_OUT_OF_MEMORY": 0x0201,
            "GRCL_ERROR_CAPACITY_EXCEEDED": 0x0202,
            "GRCL_ERROR_UNSUPPORTED_CAPABILITY": 0x0301,
            "GRCL_ERROR_INCOMPATIBLE_VERSION": 0x0302,
            "GRCL_ERROR_TIMEOUT": 0x0401,
            "GRCL_ERROR_INTERRUPTED": 0x0402,
            "GRCL_ERROR_BACKEND_FAILURE": 0x0403,
            "GRCL_ERROR_PEER_UNAVAILABLE": 0x0501,
            "GRCL_ERROR_CHANNEL_REJECTED": 0x0502,
            "GRCL_ERROR_UNKNOWN": 0x7FFF,
        }

        for name, expected in expected_values.items():
            with self.subTest(name=name):
                self.assertEqual(globals()[name], expected)

        self.assertEqual(Result.OK.value, GRCL_OK)
        self.assertEqual(Result.ERROR_BAD_STATE.value, GRCL_ERROR_BAD_STATE)

        self.assertEqual(Result.ERROR_INVALID_ARGUMENT.value, GRCL_ERROR_INVALID_ARGUMENT)
        self.assertEqual(Result.ERROR_OUT_OF_MEMORY.value, GRCL_ERROR_OUT_OF_MEMORY)
        self.assertEqual(Result.ERROR_CAPACITY_EXCEEDED.value, GRCL_ERROR_CAPACITY_EXCEEDED)
        self.assertEqual(Result.ERROR_UNSUPPORTED_CAPABILITY.value, GRCL_ERROR_UNSUPPORTED_CAPABILITY)
        self.assertEqual(Result.ERROR_INCOMPATIBLE_VERSION.value, GRCL_ERROR_INCOMPATIBLE_VERSION)
        self.assertEqual(Result.ERROR_TIMEOUT.value, GRCL_ERROR_TIMEOUT)
        self.assertEqual(Result.ERROR_INTERRUPTED.value, GRCL_ERROR_INTERRUPTED)
        self.assertEqual(Result.ERROR_BACKEND_FAILURE.value, GRCL_ERROR_BACKEND_FAILURE)
        self.assertEqual(Result.ERROR_PEER_UNAVAILABLE.value, GRCL_ERROR_PEER_UNAVAILABLE)
        self.assertEqual(Result.ERROR_CHANNEL_REJECTED.value, GRCL_ERROR_CHANNEL_REJECTED)
        self.assertEqual(Result.ERROR_UNKNOWN.value, GRCL_ERROR_UNKNOWN)


if __name__ == "__main__":
    unittest.main()
