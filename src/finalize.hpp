#ifndef FINALIZE_HPP
#define FINALIZE_HPP
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "config.hpp"

namespace RexVM {

    struct VM;
    struct OopHolder;

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

    void traceOop(ref oop);
    std::vector<ref> getGcRoots(VM &vm);
    void collect(std::vector<ref> &currentRefs, std::vector<ref> &liveRefs);
    void collect(OopHolder &oopHolder, std::vector<ref> &liveRefs);
    
}


#endif