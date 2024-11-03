# `napi_threadsafe_function` is very hard to use safely

- Node.js issue: TODO

## Description of the bug

The handle obtained with `napi_create_threadsafe_function` is supposed to be usable by arbitrary threads.
In particular, the following operations should be safe to call from arbitrary threads:
`napi_get_threadsafe_function_context`, `napi_call_threadsafe_function`, `napi_acquire_threadsafe_function`, and `napi_release_threadsafe_function`.

However, this is currently not the case.
In particular in the face of Node.js environment shutdown there are data races and use-after-frees that occur when threads call any of these functions during or after the cleanup that happens when the Node.js environment shuts down.

There are two main issues I already found:
- During finalization, the queue is accessed without holding its mutex [here](https://github.com/nodejs/node/blob/b38e3124862f7623fd8e8f0a9b738065d9296a74/src/node_api.cc#L300)
    - If another thread calls `napi_call_threadsafe_function` concurrently, this leads to a data race on the queue internals [here](https://github.com/nodejs/node/blob/b38e3124862f7623fd8e8f0a9b738065d9296a74/src/node_api.cc#L243)
- At the end of finalization, the whole internal state is just deleted [here](https://github.com/nodejs/node/blob/4f5db8b26d906f1cbe9f6a9ac2028b0f7ad88c91/src/node_api.cc#L303) even if there are still threads holding handles to the tsnf.
    - Afterwards each use of any of the above-mentioned functions is a use-after-free bug

## Workarounds are only technically possible 

While working around this is technically possible (see [src/run/fixed.hpp](https://github.com/mika-fischer/node-bug-napi-tsfn/blob/main/src/run_fixed.hpp)) it involves attaching a finalizer in order to track the finalization state in an external flag, whose lifetime must be managed via a shared_ptr or similar and all access to the TSFN must be protected by another mutex (or read-write-lock). This makes the whole thing very unergonomic to use and I'm pretty sure nobody will jump through these hoops.

## What should happen instead

- Finalization should lock the mutex to make concurrent calls safe.
- Finalization should put the TSFN into a state where all its relevant resources are released, but *only* delete the actual TSFN object if there are no more handles to it. Otherwise the actual deletion should be deferred until one of `napi_release_threadsafe_function` or `napi_call_threadsafe_function` decreases the thread_count to zero.