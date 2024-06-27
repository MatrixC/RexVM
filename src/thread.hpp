//
// Created by ch on 12/31/23.
//

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <memory>
#include <vector>
#include <cstdint>
#include <thread>
#include "config.hpp"
#include "basic_type.hpp"

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
        ThreadStatusEnum status{ThreadStatusEnum::Init};
        VM &vm;
        ThreadOop *vmThread;
        Frame *currentFrame{nullptr};

        explicit Thread(VM &vm);
        ~Thread();

        [[nodiscard]] ThreadOop *getThreadMirror() const;
        [[nodiscard]] std::vector<Oop *> getThreadGCRoots() const;

    };

}

#endif
