#pragma once

#include "utils.hpp"

namespace addon::broken {

void thread_func(napi_threadsafe_function tsfn) {
  fprintf(stderr, "thread_func: starting\n");
  auto status =
      napi_call_threadsafe_function(tsfn, nullptr, napi_tsfn_blocking);
  while (status == napi_ok) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    status = napi_call_threadsafe_function(tsfn, nullptr, napi_tsfn_blocking);
  }
  fprintf(stderr, "thread_func: Got status %d, exiting...\n", status);
}

void tsfn_callback(napi_env env, napi_value js_cb, void *ctx, void *data) {
  if (env == nullptr) {
    fprintf(stderr, "tsfn_callback: env=%p\n", env);
  }
}

void tsfn_finalize(napi_env env, void *finalize_data, void *finalize_hint) {
  fprintf(stderr, "tsfn_finalize: env=%p\n", env);
}

std::vector<std::jthread> threads;

auto run(napi_env env, napi_callback_info info) -> napi_value {
  auto global = NAPI_CALL(napi_value, napi_get_global, env);
  auto undefined = NAPI_CALL(napi_value, napi_get_undefined, env);
  auto n_threads = 32;
  auto tsfn =
      NAPI_CALL(napi_threadsafe_function, napi_create_threadsafe_function, env,
                nullptr, global, undefined, 0, n_threads, nullptr,
                tsfn_finalize, nullptr, tsfn_callback);
  for (auto i = 0; i < n_threads; ++i) {
    threads.emplace_back([tsfn] { thread_func(tsfn); });
  }
  NAPI_CALL(void, napi_unref_threadsafe_function, env, tsfn);
  return NAPI_CALL(napi_value, napi_get_undefined, env);
}

} // namespace addon::broken