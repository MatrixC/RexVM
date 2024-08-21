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
#include <unordered_map>
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

    //wait 100ms
    constexpr size_t GC_STOP_WAIT_TIME_OUT = 50;

    constexpr size_t GC_ROOT_START_SIZE = 8192;
    //10MB
    constexpr size_t GC_MEMORY_THRESHOLD = 1;
    //Frequency 2s
    constexpr size_t GC_SLEEP_TIME = 500;

    struct FinalizeRunner {

        explicit FinalizeRunner(VM &vm, GarbageCollect &collector);

        GarbageCollect &collector;
        std::mutex dequeMtx;
        std::condition_variable cv;
        std::deque<InstanceOop *> oopDeque;
        VMThread *finalizeThread{nullptr};

        void add(InstanceOop *oop);
        void runOopFinalize(InstanceOop *oop) const;
        bool runOneOop();
        void runnerMethod();
        void initFinalizeThread(VMThread *mainThread);

    };

    struct GarbageCollectContext {
        i8 startTime{};
        i8 getGcRootEndTime{};
        i8 traceOopEndTime{};
        i8 endTime{};

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
        size_t collectMemoryThreshold{GC_MEMORY_THRESHOLD};
        size_t collectStopWaitTimeout{GC_STOP_WAIT_TIME_OUT};
        size_t collectSleepTime{GC_SLEEP_TIME};
        size_t sumCollectedMemory{0};
        size_t collectStartCount{0};
        size_t collectSuccessCount{0};

        bool enableLog{true};
        bool enableGC{true};

        Class *stringClass{nullptr};

        void checkStopForCollect(VMThread &thread);
        bool checkTerminationCollect();
        bool stopTheWorld();
        void startTheWorld();
        void run();

        void getClassStaticRef(std::vector<ref> &gcRoots) const;
        void getThreadRef(std::vector<ref> &gcRoots);
        std::vector<ref> getGarbageCollectRoots();

        void traceMarkOop(ref oop) const;
        void traceMarkInstanceOopChild(InstanceOop *oop) const;
        void traceMarkObjArrayOopChild(ObjArrayOop * oop) const;

        void collectOopHolder(OopHolder &holder, GarbageCollectContext &context);
        void deleteOop(ref oop);
        void collectAll();

        void start();
        void join();
    };

#ifdef DEBUG
    extern std::unordered_map<ref, cstring> collectedOopDesc;
    cstring getCollectedOopDesc(ref oop);
#endif


}

#endif