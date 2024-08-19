#ifndef THREAD_HPP
#define THREAD_HPP

#include <vector>
#include <memory>
#include <thread>
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

        Frame *currentFrame{nullptr};
        std::atomic_bool interrupted{false};
        volatile bool stopForCollect{false};
        OopHolder oopHolder;

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
        void start(Frame *currentFrame_, bool addToThreadDeque = true);
        void join();

        void setStatus(ThreadStatusEnum status);
        [[nodiscard]] ThreadStatusEnum getStatus() const;
        [[nodiscard]] bool isDaemon() const;
        [[nodiscard]] bool isAlive() const;
        void getThreadGCRoots(std::vector<ref> &result) const;
        void getCollectRoots(std::vector<ref> &result) const;
        [[nodiscard]] bool hasNativeCall() const;
        

        private:
            void run();

    };

}

#endif
