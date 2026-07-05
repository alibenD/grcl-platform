#ifndef GRCL_CPP_SUBSCRIPTION_HPP_
#define GRCL_CPP_SUBSCRIPTION_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>

namespace grcl {

class Subscription {
 public:
  Subscription() noexcept = default;
  explicit Subscription(::grcl_subscription_t * handle) noexcept
      : handle_(handle) {}

  Subscription(const Subscription &) = delete;
  Subscription & operator=(const Subscription &) = delete;

  Subscription(Subscription && other) noexcept
      : handle_(other.release()) {}

  Subscription & operator=(Subscription && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Subscription() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Subscription * out,
    Node * node,
    const ::grcl_subscription_options_t * options) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_subscription_t * handle = nullptr;
    const Result result = ::grcl_subscription_create(node->get(), options, &handle);
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

    const Result result = ::grcl_subscription_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result take_bytes(
    void * out_payload,
    size_t payload_capacity,
    size_t * out_payload_size) noexcept
  {
    return ::grcl_subscription_take_bytes(
      handle_,
      out_payload,
      payload_capacity,
      out_payload_size);
  }

  [[nodiscard]] Result get_endpoint(::grcl_endpoint_t ** out_endpoint) const noexcept
  {
    return ::grcl_subscription_get_endpoint(handle_, out_endpoint);
  }

  [[nodiscard]] ::grcl_subscription_t * get() const noexcept
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

  [[nodiscard]] ::grcl_subscription_t * release() noexcept
  {
    ::grcl_subscription_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Subscription & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_subscription_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_subscription_t * handle_ = nullptr;
};

inline void swap(Subscription & lhs, Subscription & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_SUBSCRIPTION_HPP_
