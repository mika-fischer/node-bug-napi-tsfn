#pragma once

#include "utils.hpp"

namespace addon::fixed {

struct State {
  mutable std::shared_mutex mutex;
  napi_threadsafe_function tsfn;
};

void thread_func(int sleep, const State &state) {
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
  //   fprintf(stderr, "thread_func: starting\n");
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto lock = std::shared_lock(state.mutex);
    if (state.tsfn == nullptr) {
      fprintf(stderr, "thread_func: Got nullptr, exiting...\n");
      return;
    }
    auto status =
        napi_call_threadsafe_function(state.tsfn, nullptr, napi_tsfn_blocking);
    if (status != napi_ok) {
      fprintf(stderr, "thread_func: Got status %d, exiting...\n", status);
      return;
    }
  }
}

void tsfn_callback(napi_env env, napi_value js_cb, void *ctx, void *data) {
  //   fprintf(stderr, "tsfn_callback: env=%p\n", env);
}

void tsfn_finalize(napi_env env, void *finalize_data, void *finalize_hint) {
  fprintf(stderr, "tsfn_finalize: env=%p\n", env);
  auto state_ptr = std::unique_ptr<std::shared_ptr<State>>(
      static_cast<std::shared_ptr<State> *>(finalize_data));
  auto &state = **state_ptr;
  auto lock = std::unique_lock(state.mutex);
  fprintf(stderr, "tsfn_finalize: setting tsfn to nullptr\n");
  state.tsfn = nullptr;
}

std::vector<std::jthread> threads;

auto run(napi_env env, napi_callback_info info) -> napi_value {
  auto global = NAPI_CALL(napi_value, napi_get_global, env);
  auto undefined = NAPI_CALL(napi_value, napi_get_undefined, env);
  auto n_threads = 32;
  auto state = std::make_shared<State>();
  auto state_ptr = std::make_unique<std::shared_ptr<State>>(state);
  state->tsfn =
      NAPI_CALL(napi_threadsafe_function, napi_create_threadsafe_function, env,
                nullptr, global, undefined, 0, n_threads, state_ptr.get(),
                tsfn_finalize, nullptr, tsfn_callback);
  (void)state_ptr.release();
  for (auto i = 0; i < n_threads; ++i) {
    threads.emplace_back([i, state] { thread_func(i, *state); });
  }
  NAPI_CALL(void, napi_unref_threadsafe_function, env, state->tsfn);
  return NAPI_CALL(napi_value, napi_get_undefined, env);
}

} // namespace addon::fixed