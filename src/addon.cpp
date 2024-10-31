#include "run_broken.hpp"
#include "run_fixed.hpp"
#include "utils.hpp"

namespace addon {

napi_value init(napi_env env, napi_value exports) {
  auto run_broken_func = NAPI_CALL(napi_value, napi_create_function, env,
                                   nullptr, 0, broken::run, nullptr);
  NAPI_CALL(void, napi_set_named_property, env, exports, "run_broken",
            run_broken_func);
  auto run_fixed_func = NAPI_CALL(napi_value, napi_create_function, env,
                                  nullptr, 0, fixed::run, nullptr);
  NAPI_CALL(void, napi_set_named_property, env, exports, "run_fixed",
            run_fixed_func);
  return exports;
}

} // namespace addon

// Register the module with Node.js
NAPI_MODULE(NODE_GYP_MODULE_NAME, ::addon::init)