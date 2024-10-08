#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <deque>
#include <chrono>
#include "config.hpp"
#include "utils/spin_lock.hpp"

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

    struct ApplicationParameter {
        cstring userClassPath;
        std::vector<cstring> userParams;
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
        void exitVM();
    };

    void vmMain(ApplicationParameter &param);

}

#endif