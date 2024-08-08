#ifndef COMPILE_CLANG_HPP
#define COMPILE_CLANG_HPP

#define ATTR_UNUSED __attribute__((unused))

#define LOAD_FENCE_IMPL __atomic_signal_fence(__ATOMIC_ACQUIRE);
#define STORE_FENCE_IMPL __atomic_signal_fence(__ATOMIC_RELEASE);
#define FULL_FENCE_IMPL __atomic_thread_fence(__ATOMIC_SEQ_CST);

#endif