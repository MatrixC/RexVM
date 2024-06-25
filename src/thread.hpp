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

    struct Thread {
        VM &vm;
        std::thread systemThread;
        // std::unique_ptr<ThreadOop> vmThread;
        // std::unique_ptr<InstanceOop> vmThreadGroup;
        ThreadOop *vmThread;
        Frame *currentFrame{nullptr};

        explicit Thread(VM &vm);
        ~Thread();

        [[nodiscard]] ThreadOop *getThreadMirror() const;
        [[nodiscard]] std::vector<Oop *> getThreadGCRoots() const;

        void join();
        
    };

}

#endif
