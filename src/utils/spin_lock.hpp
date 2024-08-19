#ifndef SPINLOCK_HPP
#define SPINLOCK_HPP

#include <atomic>

namespace RexVM {

    struct SpinLock {
        explicit SpinLock() = default;

        std::atomic_flag locked ATOMIC_FLAG_INIT;

        void lock() {
            while (locked.test_and_set(std::memory_order_acquire)) {
            }
        }

        void unlock() {
            locked.clear(std::memory_order_release);
        }
    };
}

#endif
