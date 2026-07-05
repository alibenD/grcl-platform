#ifndef GRCL_CPP_EXECUTOR_HPP_
#define GRCL_CPP_EXECUTOR_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace grcl {

class Executor {
 public:
  Executor() noexcept = default;
  explicit Executor(::grcl_executor_t * handle) noexcept
      : handle_(handle) {}

  Executor(const Executor &) = delete;
  Executor & operator=(const Executor &) = delete;

  Executor(Executor && other) noexcept
      : handle_(other.release()) {}

  Executor & operator=(Executor && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Executor() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Executor * out,
    Runtime * runtime,
    const ::grcl_executor_options_t * options = nullptr) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || runtime == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_executor_t * handle = nullptr;
    const Result result = ::grcl_executor_create(runtime->get(), options, &handle);
    if (result == GRCL_OK) {
      out->handle_ = handle;
    }
    return result;
  }

  [[nodiscard]] Result destroy() noexcept
  {
    if (handle_ == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    const Result result = ::grcl_executor_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result add_node(Node * node) noexcept
  {
    if (node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }
    return ::grcl_executor_add_node(handle_, node->get());
  }

  [[nodiscard]] Result remove_node(Node * node) noexcept
  {
    if (node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }
    return ::grcl_executor_remove_node(handle_, node->get());
  }

  [[nodiscard]] Result spin_once(uint64_t timeout_ns) noexcept
  {
    return ::grcl_executor_spin_once(handle_, timeout_ns);
  }

  [[nodiscard]] ::grcl_executor_t * get() const noexcept
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

  [[nodiscard]] ::grcl_executor_t * release() noexcept
  {
    ::grcl_executor_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Executor & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_executor_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_executor_t * handle_ = nullptr;
};

inline void swap(Executor & lhs, Executor & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_EXECUTOR_HPP_
