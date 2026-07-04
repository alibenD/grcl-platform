#ifndef GRCL_CPP_RUNTIME_HPP_
#define GRCL_CPP_RUNTIME_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/result.hpp>

namespace grcl {

class Runtime {
 public:
  Runtime() noexcept = default;
  explicit Runtime(::grcl_runtime_t * handle) noexcept
      : handle_(handle) {}

  Runtime(const Runtime &) = delete;
  Runtime & operator=(const Runtime &) = delete;

  Runtime(Runtime && other) noexcept
      : handle_(other.release()) {}

  Runtime & operator=(Runtime && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Runtime() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Runtime * out,
    const ::grcl_runtime_options_t * options = nullptr) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_runtime_t * handle = nullptr;
    const Result result = ::grcl_runtime_create(options, &handle);
    if (result == GRCL_OK) {
      out->handle_ = handle;
    }
    return result;
  }

  [[nodiscard]] static Result init_with_storage(
    Runtime * out,
    const ::grcl_runtime_options_t * options,
    const ::grcl_storage_t * storage) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_runtime_t * handle = nullptr;
    const Result result =
      ::grcl_runtime_init_with_storage(options, storage, &handle);
    if (result == GRCL_OK) {
      out->handle_ = handle;
    }
    return result;
  }

  [[nodiscard]] Result start() noexcept
  {
    return ::grcl_runtime_start(handle_);
  }

  [[nodiscard]] Result stop() noexcept
  {
    return ::grcl_runtime_stop(handle_);
  }

  [[nodiscard]] Result destroy() noexcept
  {
    if (handle_ == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    const Result result = ::grcl_runtime_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] ::grcl_runtime_t * get() const noexcept
  {
    return handle_;
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return handle_ == nullptr;
  }

  explicit operator bool() const noexcept
  {
    return !empty();
  }

  [[nodiscard]] ::grcl_runtime_t * release() noexcept
  {
    ::grcl_runtime_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Runtime & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_runtime_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_runtime_t * handle_ = nullptr;
};

inline void swap(Runtime & lhs, Runtime & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_RUNTIME_HPP_
