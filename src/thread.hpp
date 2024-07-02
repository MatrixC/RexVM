//
// Created by ch on 12/31/23.
//

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <vector>
#include <memory>
#include <thread>
#include "config.hpp"
#include "class.hpp"
#include "oop.hpp"

namespace RexVM {

    struct VM;
    struct ClassLoader;
    struct Frame;
    struct Method;
    struct ThreadOop;
    struct InstanceOop;
    struct Oop;

    enum class ThreadStatusEnum {
        Init,
        Running,
        Terminated,
    };

    struct Thread {
        ThreadStatusEnum status{ThreadStatusEnum::Running};
        std::thread nativeThread;
        ThreadOop *vmThreadOop;
        Frame *currentFrame{nullptr};

        cstring name;
        VM &vm;

        std::unique_ptr<Slot[]> stackMemory;
        std::unique_ptr<SlotTypeEnum[]> stackMemoryType;

        explicit Thread(VM &vm, Method &method, std::vector<Slot> params, bool mainThread);
        ~Thread();

        [[nodiscard]] ThreadOop *getThreadMirror() const;
        [[nodiscard]] std::vector<Oop *> getThreadGCRoots() const;

    };

    struct VMThread : InstanceOop {
        bool isMainThread{false};
        std::thread nativeThread;
        Frame *currentFrame{nullptr};
        Method &runMethod;
        std::vector<Slot> params;
        std::unique_ptr<Slot[]> stackMemory;
        std::unique_ptr<SlotTypeEnum[]> stackMemoryType;

        VM &vm;

        explicit VMThread(VM &vm, Method *runnableMethod, std::vector<Slot> runnableMethodParams);
        ~VMThread();

        void start();

        cstring getName() const;
        std::vector<Oop *> getThreadGCRoots() const;

    };

}

#endif
