#pragma once

#include <js_native_api.h>
#include <node_api.h>
#include <node_api_types.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace addon {

template <typename R, auto func, typename... Args>
inline auto call(const char *name, Args &&...args) -> R {
  napi_status status;
  if constexpr (std::is_same_v<R, void>) {
    status = func(std::forward<Args>(args)...);
    if (status == napi_ok) {
      return;
    }
  } else {
    R ret;
    status = func(std::forward<Args>(args)..., &ret);
    if (status == napi_ok) {
      return ret;
    }
  }
  std::fprintf(stderr, "%s: %d\n", name, status);
  std::abort();
}

} // namespace addon

#define NAPI_CALL(ret_type, func, ...)                                         \
  ::addon::call<ret_type, func>(#func, ##__VA_ARGS__)
