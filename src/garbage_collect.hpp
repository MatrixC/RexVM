#ifndef GARBAGE_COLLECT_HPP
#define GARBAGE_COLLECT_HPP
#include "config.hpp"
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

namespace RexVM {

    struct VM;
    struct VMThread;
    struct Frame;
    struct Class;
    struct InstanceOop;
    struct ObjArrayOop;

    constexpr size_t GC_ROOT_START_SIZE = 8192;
    constexpr size_t GC_SURVIVE_SIZE = 10240;

    struct GarbageCollect {

        explicit GarbageCollect(VM &vm);

        VM &vm;
        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;
        std::thread gcThread;
        bool enableLog{true};

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

        void collect(ref oop, std::vector<ref> &survives, const std::vector<ref>& ref) const;
        void collectAll() const;

        void start();
        void join();
    };

}

#endif