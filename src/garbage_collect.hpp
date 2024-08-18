#ifndef GARBAGE_COLLECT_HPP
#define GARBAGE_COLLECT_HPP
#include "config.hpp"
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <deque>
#include <condition_variable>

namespace RexVM {

    struct VM;
    struct VMThread;
    struct Frame;
    struct Class;
    struct InstanceOop;
    struct ObjArrayOop;
    struct OopHolder;
    struct GarbageCollect;

    constexpr size_t GC_ROOT_START_SIZE = 8192;
    //10MB
    constexpr size_t GC_MEMORY_THRESHOLD = 10 * 1024 * 1024;
    //Frequency 2s
    constexpr size_t GC_SLEEP_TIME = 2000;

    struct FinalizeRunner {

        explicit FinalizeRunner(VM &vm, GarbageCollect &collector);

        GarbageCollect &collector;
        std::unique_ptr<VMThread> thread;
        std::mutex dequeMtx;
        std::condition_variable cv;
        std::deque<InstanceOop *> oopDeque;

        void add(InstanceOop *oop);
        void run(InstanceOop *oop) const;
        void start();

    };

    struct GarbageCollectContext {
        std::chrono::system_clock::time_point startTime{std::chrono::system_clock::now()};
        std::chrono::system_clock::time_point getGcRootEndTime;
        std::chrono::system_clock::time_point traceOopEndTime;
        std::chrono::system_clock::time_point endTime;

        explicit GarbageCollectContext(VM &vm);

        size_t allocatedOopCount{0};
        size_t allocatedOopMemory{0};
        std::atomic_size_t collectedOopCount{0};
        std::atomic_size_t collectedOopMemory{0};

        void endGetRoots();
        void endTraceOop();

        void collectFinish(VM &vm);
        void printLog(VM &vm) const;
    };

    struct GarbageCollect {

        explicit GarbageCollect(VM &vm);

        VM &vm;
        FinalizeRunner finalizeRunner;
        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;
        std::thread gcThread;
        bool enableLog{true};
        size_t collectMemoryThreshold{GC_MEMORY_THRESHOLD};
        size_t collectSleepTime{GC_SLEEP_TIME};
        size_t sumCollectedMemory{0};

        Class *stringClass{nullptr};

        void checkStopForCollect(VMThread &thread);
        bool checkTerminationCollect();
        void stopTheWorld();
        void startTheWorld();
        void run();

        void getClassStaticRef(std::vector<ref> &gcRoots) const;
        void getThreadRef(std::vector<ref> &gcRoots);
        std::vector<ref> getGarbageCollectRoots();

        void traceMarkOop(ref oop) const;
        void traceMarkInstanceOopChild(InstanceOop *oop) const;
        void traceMarkObjArrayOopChild(ObjArrayOop * oop) const;

        void collectOopHolder(OopHolder &holder, GarbageCollectContext &context);
        void collectAll() const;

        void start();
        void join();
    };


}

#endif