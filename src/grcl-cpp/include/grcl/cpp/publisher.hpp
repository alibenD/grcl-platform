#ifndef GRCL_CPP_PUBLISHER_HPP_
#define GRCL_CPP_PUBLISHER_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>

namespace grcl {

class Publisher {
 public:
  Publisher() noexcept = default;
  explicit Publisher(::grcl_publisher_t * handle) noexcept
      : handle_(handle) {}

  Publisher(const Publisher &) = delete;
  Publisher & operator=(const Publisher &) = delete;

  Publisher(Publisher && other) noexcept
      : handle_(other.release()) {}

  Publisher & operator=(Publisher && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Publisher() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Publisher * out,
    Node * node,
    const ::grcl_publisher_options_t * options) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_publisher_t * handle = nullptr;
    const Result result = ::grcl_publisher_create(node->get(), options, &handle);
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

    const Result result = ::grcl_publisher_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result publish_bytes(
    const void * payload,
    size_t payload_size) noexcept
  {
    return ::grcl_publisher_publish_bytes(handle_, payload, payload_size);
  }

  [[nodiscard]] Result get_endpoint(::grcl_endpoint_t ** out_endpoint) const noexcept
  {
    return ::grcl_publisher_get_endpoint(handle_, out_endpoint);
  }

  [[nodiscard]] ::grcl_publisher_t * get() const noexcept
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

  [[nodiscard]] ::grcl_publisher_t * release() noexcept
  {
    ::grcl_publisher_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Publisher & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_publisher_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_publisher_t * handle_ = nullptr;
};

inline void swap(Publisher & lhs, Publisher & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_PUBLISHER_HPP_
