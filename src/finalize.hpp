#ifndef FINALIZE_HPP
#define FINALIZE_HPP
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include "config.hpp"

namespace RexVM {

    struct VM;
    struct OopHolder;
    struct ClassLoader;
    struct Frame;

    struct Collector {

        VM &vm;

        bool markCollect{false};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic_size_t stopThreadCount{0};
        std::thread collectThread;

        explicit Collector(VM &vm);

        void stopTheWorld();

        void collectFinish();

        void checkStop(Frame &frame, const cstring &methodName);

        void startGC();

        void join();


    };


        void deleteItem(ref item);

        void showItem(ref item);

    void traceOop(ref oop);
    std::vector<ref> getGcRoots(VM &vm);
    void collect(VM &vm, std::vector<ref> &currentRefs, std::vector<ref> &liveRefs, u8 &deleteCnt);
    void collect(VM &vm, OopHolder &oopHolder, std::vector<ref> &liveRefs, u8 &deleteCnt);

    void getStaticRef(ClassLoader &classLoader, std::vector<ref> &gcRoots);
    void getThreadRef(VM &vm, std::vector<ref> &gcRoots);
    
}


#endif