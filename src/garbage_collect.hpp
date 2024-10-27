#ifndef GARBAGE_COLLECT_HPP
#define GARBAGE_COLLECT_HPP
#include "basic.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <deque>
#include <condition_variable>
#include <hash_table8.hpp>

namespace RexVM {

    struct VM;
    struct VMThread;
    struct Frame;
    struct Class;
    struct InstanceOop;
    struct ObjArrayOop;
    struct OopHolder;
    struct GarbageCollect;

    struct FinalizeRunner {

        explicit FinalizeRunner(VM &vm, GarbageCollect &collector);

        GarbageCollect &collector;
        std::mutex dequeMtx;
        std::condition_variable cv;
        std::deque<InstanceOop *> oopDeque;
        VMThread *finalizeThread{nullptr};

        void initFinalizeThread(VMThread *mainThread);
        void add(InstanceOop *oop);

    private:
        
        void runOopFinalize(InstanceOop *oop) const;
        bool runOneOop();
        void runnerMethod();
        

    };

    struct GarbageCollectContext {
        i8 startTime{};
        i8 getGcRootEndTime{};
        i8 traceOopEndTime{};
        i8 endTime{};

        explicit GarbageCollectContext(const VM &vm);

        size_t tempAllocatedOopCount{0};
        size_t tempAllocatedOopMemory{0};
        std::atomic_size_t collectedOopCount{0};
        std::atomic_size_t collectedOopMemory{0};

        void endGetRoots();
        void endTraceOop();

        void collectFinish(const VM &vm);
        void printLog(const VM &vm) const;
    };

    struct GarbageCollect {

        explicit GarbageCollect(VM &vm);

        VM &vm;
        FinalizeRunner finalizeRunner;
        bool checkStop{false};
        std::mutex checkStopMtx;
        std::condition_variable checkStopCv;
        bool notifyCollect{false};
        std::mutex notifyCollectMtx;
        std::condition_variable notifyCollectCv;
        std::thread gcThread;
        size_t gcRootReserveSize;
        size_t collectMemoryThreshold;
        size_t collectStopWaitTimeout;
        size_t collectSleepTime;
        size_t sumCollectedMemory{0};
        size_t collectStartCount{0};
        size_t collectSuccessCount{0};

        bool enableGC;
        bool enableLog{true};
        bool enableFinalize{false};

        Class *stringClass{nullptr};

        void checkStopForCollect(VMThread &thread);
        void start();
        void join();
        void notify();
        void collectAll();

    private:
        
        [[nodiscard]] bool checkTerminationCollect() const;
        bool stopTheWorld();
        void startTheWorld();
        void run();
        

        void getClassStaticRef(std::vector<ref> &gcRoots) const;
        void getThreadRef(std::vector<ref> &gcRoots) const;
        [[nodiscard]] std::vector<ref> getGarbageCollectRoots() const;
        [[nodiscard]] std::vector<OopHolder *> getHolders() const;

        void process();

        void processTrace(GarbageCollectContext &context);
        void traceMarkOop(ref oop) const;
        void traceMarkInstanceOopChild(const InstanceOop *oop) const;
        void traceMarkObjArrayOopChild(const ObjArrayOop * oop) const;

        void processCollect(GarbageCollectContext &context) const;
        void collectOopHolder(OopHolder &holder, GarbageCollectContext &context) const;

        void deleteOop(ref oop) const;


#ifdef DEBUG

        void recordOopCreate(Frame &frame, ref oop);

#endif

    };

#ifdef DEBUG
    extern emhash8::HashMap<ref, cview> collectedOopDesc;
    extern emhash8::HashMap<ref, cstring> collectedOopDesc2;
    extern emhash8::HashMap<ref, cstring> collectedOopDesc3;
    cview getCollectedOopDesc(ref oop);
    cstring getCollectedOopDesc2(ref oop);
#endif


}

#endif