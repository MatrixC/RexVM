#ifndef GARBAGE_COLLECT_HPP
#define GARBAGE_COLLECT_HPP
#include "config.hpp"
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace RexVM {

    struct VM;
    struct VMThread;
    struct Frame;
    struct InstanceOop;
    struct ObjArrayOop;

    constexpr size_t GC_ROOT_START_SIZE = 8192;

    struct GarbageCollect {

        VM &vm;
        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;

        void checkStorForCollect(Frame &frame);
        bool checkTerminationCollect();
        void stopTheWorld();
        void startTheWorld();
        void run();

        void getClassStaticRef(std::vector<ref> gcRoots);
        void getThreadRef(std::vector<ref> gcRoots);
        std::vector<ref> getGarbageCollectRoots();

        void traceMarkOop(ref oop) const;
        void traceMarkInstanceOopChild(InstanceOop *oop) const;
        void traceMarkObjArrayOopChild(ObjArrayOop * oop) const;


    };

}

#endif