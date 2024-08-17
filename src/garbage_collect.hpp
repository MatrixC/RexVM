#ifndef GARBAGE_COLLECT_HPP
#define GARBAGE_COLLECT_HPP
#include "config.hpp"
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

namespace RexVM {

    struct VM;
    struct VMThread;
    struct Frame;
    struct Class;
    struct InstanceOop;
    struct ObjArrayOop;
    struct OopHolder;

    constexpr size_t GC_ROOT_START_SIZE = 8192;
    constexpr size_t GC_SURVIVE_SIZE = 10240;

    struct GarbageCollectContext {
        std::chrono::system_clock::time_point startTime{std::chrono::system_clock::now()};
        std::chrono::system_clock::time_point getGcRootEndTime;
        std::chrono::system_clock::time_point traceOopEndTime;
        std::chrono::system_clock::time_point endTime;

        explicit GarbageCollectContext();

        std::atomic_size_t createdOopCount{0};
        std::atomic_size_t createdOopMemory{0};
        std::atomic_size_t collectedOopCount{0};
        std::atomic_size_t collectedOopMemory{0};
        std::atomic_size_t remainOopCount{0};
        std::atomic_size_t remainOopMemory{0};

        void endGetRoots();
        void endTraceOop();

        void collectFinish();
        void printLog() const;
    };

    struct GarbageCollect {

        explicit GarbageCollect(VM &vm);

        VM &vm;
        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;
        std::thread gcThread;
        bool enableLog{true};
        size_t sumCollectedMemory{0};

        //Class obj constant
        //ConstantPool Class
        Class *constantPoolClass{nullptr};
        Class *stringClass{nullptr};
        //StringClass

        void checkStopForCollect(Frame &frame);
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

        void collect(ref oop, std::vector<ref> &survives, size_t &collectedOopMemory) const;
        void collectOopHolder(OopHolder &holder, GarbageCollectContext &context) const;
        void collectAll() const;

        void runFinalize(Frame &frame, InstanceOop *oop) const;

        void start();
        void join();
    };

}

#endif