#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
#include <chrono>
#include "basic.hpp"

namespace RexVM {

    struct ClassPath;
    struct ClassLoader;
    struct StringPool;
    struct NativeManager;
    struct VMThread;
    struct OopManager;
    struct Method;
    struct GarbageCollect;
    struct ThreadManager;
    class LLVM_JITManager;

    constexpr size_t GC_STOP_WAIT_TIME_OUT = 100; //wait 100ms
    constexpr size_t GC_ROOT_RESERVE_SIZE = 8192;

    constexpr size_t JIT_COMPILE_OPTIMIZE_LEVEL = 0;

#ifdef DEBUG
    constexpr size_t GC_MEMORY_THRESHOLD = 0.5 * 1024 * 1024; //1M
    constexpr size_t GC_SLEEP_TIME = 500; //500ms

    constexpr size_t JIT_INVOKE_COUNT_THRESHOLD = 0;
#else
    constexpr size_t GC_MEMORY_THRESHOLD = 20 * 1024 * 1024; //20MB
    constexpr size_t GC_SLEEP_TIME = 5000; //5000ms

    constexpr size_t JIT_INVOKE_COUNT_THRESHOLD = 20;
#endif


    struct ApplicationParameter {
        cstring userClassPath;
        std::vector<cstring> userParams;

        bool gcEnable{true};
        bool gcEnableLog{false};
        bool gcEnableFinalize{false};
        size_t gcGCRootReserveSize{GC_ROOT_RESERVE_SIZE};
        size_t gcCollectMemoryThreshold{GC_MEMORY_THRESHOLD};
        size_t gcCollectStopWaitTimeout{GC_STOP_WAIT_TIME_OUT};
        size_t gcCollectSleepTime{GC_SLEEP_TIME};

        bool jitEnable{true};
        size_t jitCompileMethodInvokeCountThreshold{JIT_INVOKE_COUNT_THRESHOLD};
        size_t jitCompileOptimizeLevel{JIT_COMPILE_OPTIMIZE_LEVEL};
        bool jitLVTOptimize{true};
        bool jitCheckStack{false};
        bool jitSupportException{true};
    };

    struct VM {
        ApplicationParameter &params;
        std::unique_ptr<ClassPath> classPath;
        std::unique_ptr<OopManager> oopManager;
        std::unique_ptr<ThreadManager> threadManager;
        std::unique_ptr<StringPool> stringPool;
        std::unique_ptr<ClassLoader> bootstrapClassLoader;
        std::unique_ptr<GarbageCollect> garbageCollector;
        std::unique_ptr<VMThread> mainThread;
#ifdef LLVM_JIT
        std::unique_ptr<LLVM_JITManager> jitManager;
#endif
        std::chrono::system_clock::time_point startTime{std::chrono::system_clock::now()};
        cstring javaHome{};
        cstring javaClassPath{};
        bool exit{false};

        explicit VM(ApplicationParameter &params);

        void start();

    private:
        bool initVM();
        void runMainMethod() const;
        void joinThreads();
        void exitVM() const;
    };

    void vmMain(ApplicationParameter &param);

}

#endif