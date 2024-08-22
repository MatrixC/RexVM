#ifndef THREAD_HPP
#define THREAD_HPP

#include <vector>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include "config.hpp"
#include "oop.hpp"
#include "memory.hpp"

namespace RexVM {

    class Oop;
    struct VM;
    struct Frame;
    struct Method;
    struct InstanceOop;
    struct OopHolder;

    struct VMThreadMethod {
        explicit VMThreadMethod(Method *method, std::vector<Slot> params);
        explicit VMThreadMethod(VMTheadNativeHandler nativeMethod);
        Method *method{nullptr};
        VMTheadNativeHandler nativeMethod;
        std::vector<Slot> params;
    };

    struct VMThread : InstanceOop {
        VM &vm;
        std::thread nativeThread;
        std::vector<std::unique_ptr<VMThreadMethod>> runMethods;
        std::unique_ptr<Slot[]> stackMemory;
        std::unique_ptr<SlotTypeEnum[]> stackMemoryType;
        OopHolder oopHolder;

        Frame *currentFrame{nullptr};
        std::atomic_bool interrupted{false};
        volatile bool stopForCollect{false};
        volatile bool gcSafe{true};

#ifdef DEBUG
        cstring threadName{};
#endif

        //Normal
        explicit VMThread(VM &vm, InstanceClass * klass);

        //Main
        explicit VMThread(VM &vm);
        ~VMThread();

        void setName(const cstring &name);
        cstring getName() const;

        static VMThread *createOriginVMThread(VM &vm);

        void addMethod(Method *method, const std::vector<Slot>& params);
        void addMethod(VMTheadNativeHandler &method);
        void start(Frame *currentFrame_, bool userThread);
        void join();

        void setStatus(ThreadStatusEnum status);
        [[nodiscard]] ThreadStatusEnum getStatus() const;
        [[nodiscard]] bool isDaemon() const;
        [[nodiscard]] bool isAlive() const;
        void setDaemon(bool on);
        void getThreadGCRoots(std::vector<ref> &result) const;
        void getCollectRoots(std::vector<ref> &result) const;
        [[nodiscard]] bool hasNativeCall() const;

        void setGCSafe(bool val);
        [[nodiscard]] bool isGCSafe() const;
        

        private:
            void run();

    };

    struct ThreadManager {

        VM &vm;

        explicit ThreadManager(VM &vm);

        std::mutex threadsMtx;
        std::vector<VMThread *> threads;

        std::mutex joinListMtx;
        std::queue<VMThread *> joinThreads;

        void addThread(VMThread *thread, bool userThread);

        void joinUserThreads();
        bool checkAllThreadStopForCollect();
        std::vector<VMThread *> getThreads();
    };

    struct ThreadSafeGuard {
        //设置线程的GCSafe状态
        //在线程进入native方法时 因为native方法中会创建对象
        //但虚拟机跟踪不到native方法中创建的对象 所以就有可能在
        //[创建对象 - 使用这个对象] 这个过程中因为对用的了别的方法
        //进入gc状态而导致创建的对象还未被使用就回收 而指针还停留在
        //native方法栈中 进而导致core

        VMThread &thread;

        explicit ThreadSafeGuard(VMThread &thread);

        ~ThreadSafeGuard();
    };

}

#endif
