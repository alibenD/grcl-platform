#ifndef GRCL_CPP_NODE_HPP_
#define GRCL_CPP_NODE_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace grcl {

class Node {
 public:
  Node() noexcept = default;
  explicit Node(::grcl_node_t * handle) noexcept
      : handle_(handle) {}

  Node(const Node &) = delete;
  Node & operator=(const Node &) = delete;

  Node(Node && other) noexcept
      : handle_(other.release()) {}

  Node & operator=(Node && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Node() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Node * out,
    Runtime * runtime,
    const ::grcl_node_options_t * options) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || runtime == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_node_t * handle = nullptr;
    const Result result = ::grcl_node_create(runtime->get(), options, &handle);
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

    const Result result = ::grcl_node_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result get_runtime(::grcl_runtime_t ** out_runtime) const noexcept
  {
    return ::grcl_node_get_runtime(handle_, out_runtime);
  }

  [[nodiscard]] ::grcl_node_t * get() const noexcept
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

  [[nodiscard]] ::grcl_node_t * release() noexcept
  {
    ::grcl_node_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Node & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_node_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_node_t * handle_ = nullptr;
};

inline void swap(Node & lhs, Node & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_NODE_HPP_
