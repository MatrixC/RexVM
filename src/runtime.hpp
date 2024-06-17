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
    struct Executor;
    struct ClassLoader;
    struct Frame;
    struct Method;
    struct ThreadOop;
    struct InstanceOop;

    struct Thread {
        VM &vm;
        Executor &executor;
        std::thread systemThread;
        // std::unique_ptr<ThreadOop> vmThread;
        // std::unique_ptr<InstanceOop> vmThreadGroup;
        ThreadOop *vmThread;
        InstanceOop *vmThreadGroup;
        Frame *currentFrame{nullptr};
        std::vector<std::unique_ptr<Frame>> frames;

        explicit Thread(VM &vm, Executor &executor);
        ~Thread();

        [[nodiscard]] ThreadOop *getThreadMirror() const;

        [[nodiscard]] std::vector<Oop *> getThreadGCRoots() const;
        
    };

    struct Executor {
        VM &vm;
        std::vector<std::unique_ptr<Thread>> threads;

        explicit Executor(VM &vm);

        static void createFrameAndRunMethod(Thread &thread, Method &method_, std::vector<Slot> params, Frame *previous);
        static void executeFrame(Frame &frame, cstring methodName);


        void runStaticMethodOnNewThread(Method &method_, std::vector<Slot> params);

    };



}

#endif
