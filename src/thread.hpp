//
// Created by ch on 12/31/23.
//

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <vector>
#include <memory>
#include <thread>
#include "config.hpp"

namespace RexVM {

    struct VM;
    struct ClassLoader;
    struct Frame;
    struct Method;
    struct ThreadOop;
    struct InstanceOop;

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

}

#endif
