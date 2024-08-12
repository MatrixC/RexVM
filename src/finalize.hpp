#ifndef FINALIZE_HPP
#define FINALIZE_HPP
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace RexVM {

    struct VM;

    struct Collector {

        VM &vm;

        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic_size_t stopThreadCount{0};

        explicit Collector(VM &vm);

        void stopTheWorld();

        void collectFinish();

        void checkStop();

        void startGC();

    };
    
}


#endif