#ifndef THREAD_HPP
#define THREAD_HPP

#include <vector>
#include <memory>
#include <thread>
#include "config.hpp"
#include "class.hpp"
#include "memory.hpp"

namespace RexVM {

    struct VM;
    struct Frame;
    struct Method;
    struct InstanceOop;
    struct Oop;

    enum class ThreadStatusEnum : u2 {
        NEW = 0x0000,
        RUNNABLE = 0x0004,
        BLOCKED = 0x0400,
        WAITING = 0x0010,
        TIMED_WAITING = 0x0020,
        TERMINATED = 0x0002,
    };

    struct VMThread : InstanceOop {
        VM &vm;
        bool isMainThread{false};
        std::thread nativeThread;
        Frame *currentFrame{nullptr};
        Method &runMethod;
        std::atomic_bool interrupted{false};
        std::vector<Slot> params;
        std::unique_ptr<Slot[]> stackMemory;
        std::unique_ptr<SlotTypeEnum[]> stackMemoryType;
        OopHolder oopHolder;

        explicit VMThread(VM &vm, InstanceClass * klass, Method *runnableMethod, std::vector<Slot> runnableMethodParams);
        ~VMThread();

        
        void start(Frame *currentFrame);
        void join();

        [[nodiscard]] cstring getName() const;
        void setStatus(ThreadStatusEnum status);
        [[nodiscard]] ThreadStatusEnum getStatus() const;
        [[nodiscard]] bool isAlive() const;
        [[nodiscard]] std::vector<Oop *> getThreadGCRoots() const;

        private:
            void run();

    };

}

#endif
