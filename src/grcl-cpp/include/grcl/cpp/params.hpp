#ifndef GRCL_CPP_PARAMS_HPP_
#define GRCL_CPP_PARAMS_HPP_

#include <utility>

#include <grcl/c/runtime.h>

#include <grcl/cpp/result.hpp>
#include <grcl/cpp/runtime.hpp>

namespace grcl {

class Params {
 public:
  Params() noexcept = default;

  explicit Params(::grcl_runtime_t * handle) noexcept
      : handle_(handle) {}

  explicit Params(Runtime * runtime) noexcept
      : runtime_(runtime) {}

  [[nodiscard]] Result set(const ::grcl_param_record_t * param) const noexcept
  {
    return ::grcl_runtime_param_set(resolve_handle(), param);
  }

  [[nodiscard]] Result get(
    const char * name,
    ::grcl_param_record_t * out_param,
    void * value_buffer,
    size_t value_buffer_capacity,
    size_t * out_value_size) const noexcept
  {
    return ::grcl_runtime_param_get(
      resolve_handle(),
      name,
      out_param,
      value_buffer,
      value_buffer_capacity,
      out_value_size);
  }

  [[nodiscard]] Result list(
    char * out_names,
    size_t names_capacity,
    size_t * out_names_size,
    size_t * out_param_count) const noexcept
  {
    return ::grcl_runtime_param_list(
      resolve_handle(),
      out_names,
      names_capacity,
      out_names_size,
      out_param_count);
  }

  [[nodiscard]] ::grcl_runtime_t * get() const noexcept
  {
    return resolve_handle();
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return handle_ == nullptr;
  }

  explicit operator bool() const noexcept
  {
    return !empty();
  }

  void reset(::grcl_runtime_t * handle = nullptr) noexcept
  {
    runtime_ = nullptr;
    handle_ = handle;
  }

  void reset(Runtime * runtime) noexcept
  {
    runtime_ = runtime;
    handle_ = nullptr;
  }

  void swap(Params & other) noexcept
  {
    std::swap(runtime_, other.runtime_);
    std::swap(handle_, other.handle_);
  }

 private:
  [[nodiscard]] ::grcl_runtime_t * resolve_handle() const noexcept
  {
    return runtime_ == nullptr ? handle_ : runtime_->get();
  }

  Runtime * runtime_ = nullptr;
  ::grcl_runtime_t * handle_ = nullptr;
};

inline void swap(Params & lhs, Params & rhs) noexcept
{
  lhs.swap(rhs);
}

}  // namespace grcl

#endif  // GRCL_CPP_PARAMS_HPP_
