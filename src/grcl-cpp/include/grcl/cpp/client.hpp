#ifndef GRCL_CPP_CLIENT_HPP_
#define GRCL_CPP_CLIENT_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/node.hpp>
#include <grcl/cpp/result.hpp>

namespace grcl {

class Client {
 public:
  Client() noexcept = default;
  explicit Client(::grcl_client_t * handle) noexcept
      : handle_(handle) {}

  Client(const Client &) = delete;
  Client & operator=(const Client &) = delete;

  Client(Client && other) noexcept
      : handle_(other.release()) {}

  Client & operator=(Client && other) noexcept
  {
    if (this != &other) {
      close_noexcept();
      handle_ = other.release();
    }
    return *this;
  }

  ~Client() noexcept
  {
    close_noexcept();
  }

  [[nodiscard]] static Result create(
    Client * out,
    Node * node,
    const ::grcl_client_options_t * options) noexcept
  {
    if (out == nullptr || out->handle_ != nullptr || node == nullptr) {
      return GRCL_ERROR_INVALID_ARGUMENT;
    }

    ::grcl_client_t * handle = nullptr;
    const Result result = ::grcl_client_create(node->get(), options, &handle);
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

    const Result result = ::grcl_client_destroy(handle_);
    if (result == GRCL_OK) {
      handle_ = nullptr;
    }
    return result;
  }

  [[nodiscard]] Result get_endpoint(::grcl_endpoint_t ** out_endpoint) const noexcept
  {
    return ::grcl_client_get_endpoint(handle_, out_endpoint);
  }

  [[nodiscard]] Result send_request_bytes(
    const void * request_payload,
    size_t request_payload_size,
    ::grcl_request_id_t * out_request_id) noexcept
  {
    return ::grcl_client_send_request_bytes(
      handle_,
      request_payload,
      request_payload_size,
      out_request_id);
  }

  [[nodiscard]] Result take_response_bytes(
    ::grcl_request_id_t request_id,
    void * out_response_payload,
    size_t response_payload_capacity,
    size_t * out_response_payload_size) noexcept
  {
    return ::grcl_client_take_response_bytes(
      handle_,
      request_id,
      out_response_payload,
      response_payload_capacity,
      out_response_payload_size);
  }

  [[nodiscard]] ::grcl_client_t * get() const noexcept
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

  [[nodiscard]] ::grcl_client_t * release() noexcept
  {
    ::grcl_client_t * const handle = handle_;
    handle_ = nullptr;
    return handle;
  }

  void swap(Client & other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

 private:
  void close_noexcept() noexcept
  {
    if (handle_ != nullptr) {
      (void)::grcl_client_destroy(handle_);
      handle_ = nullptr;
    }
  }

  ::grcl_client_t * handle_ = nullptr;
};

inline void swap(Client & lhs, Client & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_CLIENT_HPP_
