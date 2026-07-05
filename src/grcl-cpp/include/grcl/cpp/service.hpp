#ifndef GRCL_CPP_SERVICE_HPP_
#define GRCL_CPP_SERVICE_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>

namespace grcl {

class Service {
 public:
  Service() noexcept = default;
  explicit Service(::grcl_service_t * handle) noexcept
      : handle_(handle) {}

  Service(const Service &) = delete;
  Service & operator=(const Service &) = delete;

  Service(Service && other) noexcept
      : handle_(other.release()) {}

  Service & operator=(Service && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Service() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Service * out,
    Node * node,
    const ::grcl_service_options_t * options) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_service_t * handle = nullptr;
    const Result result = ::grcl_service_create(node->get(), options, &handle);
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

    const Result result = ::grcl_service_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result get_endpoint(::grcl_endpoint_t ** out_endpoint) const noexcept
  {
    return ::grcl_service_get_endpoint(handle_, out_endpoint);
  }

  [[nodiscard]] Result take_request_bytes(
    void * out_request_payload,
    size_t request_payload_capacity,
    size_t * out_request_payload_size,
    ::grcl_request_id_t * out_request_id) noexcept
  {
    return ::grcl_service_take_request_bytes(
      handle_,
      out_request_payload,
      request_payload_capacity,
      out_request_payload_size,
      out_request_id);
  }

  [[nodiscard]] Result send_response_bytes(
    ::grcl_request_id_t request_id,
    const void * response_payload,
    size_t response_payload_size) noexcept
  {
    return ::grcl_service_send_response_bytes(
      handle_,
      request_id,
      response_payload,
      response_payload_size);
  }

  [[nodiscard]] ::grcl_service_t * get() const noexcept
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

  [[nodiscard]] ::grcl_service_t * release() noexcept
  {
    ::grcl_service_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Service & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_service_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_service_t * handle_ = nullptr;
};

inline void swap(Service & lhs, Service & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_SERVICE_HPP_
