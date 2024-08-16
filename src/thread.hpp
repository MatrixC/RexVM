#ifndef THREAD_HPP
#define THREAD_HPP

#include <vector>
#include <memory>
#include <thread>
#include "config.hpp"
#include "oop.hpp"
#include "memory.hpp"

namespace RexVM {

    struct VM;
    struct Frame;
    struct Method;
    struct InstanceOop;
    struct Oop;
    struct OopHolder;

    struct VMThread : InstanceOop {
        VM &vm;
        std::thread nativeThread;
        Frame *currentFrame{nullptr};
        Method *runMethod;
        std::atomic_bool interrupted{false};
        std::vector<Slot> params;
        std::unique_ptr<Slot[]> stackMemory;
        std::unique_ptr<SlotTypeEnum[]> stackMemoryType;
        bool stopForCollect{false};
        OopHolder oopHolder;

        //Normal
        explicit VMThread(VM &vm, InstanceClass * const klass);

        //Main
        explicit VMThread(VM &vm);


        ~VMThread();

        void reset(Method *method, std::vector<Slot> params);

        
        void start(Frame *currentFrame);
        void join();

        [[nodiscard]] cstring getName() const;
        void setStatus(ThreadStatusEnum status);
        [[nodiscard]] ThreadStatusEnum getStatus() const;
        [[nodiscard]] bool isAlive() const;
        void getThreadGCRoots(std::vector<ref> &result) const;
        void getCollectRoots(std::vector<ref> &result) const;
        bool hasNativeCall() const;

        private:
            void run();

    };

}

#endif
