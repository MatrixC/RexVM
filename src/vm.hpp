#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <deque>
#include "config.hpp"

namespace RexVM {

    struct ClassPath;
    struct ClassLoader;
    struct StringPool;
    struct NativeManager;
    struct OopManager;
    struct Thread;

    struct ApplicationParameter {
        cstring userClassPath;
        std::vector<cstring> userParams;
    };

    struct VM {
        ApplicationParameter &params;
        std::unique_ptr<ClassPath> classPath;
        std::unique_ptr<OopManager> oopManager;
        std::unique_ptr<StringPool> stringPool;
        std::unique_ptr<ClassLoader> bootstrapClassLoader;

        std::mutex threadMtx, vmThreadMtx;
        std::deque<std::thread> threadDeque;
        std::vector<Thread *> vmThreads;

        explicit VM(ApplicationParameter &params);

        void start();
        void addVMThread(Thread *thread);
        void removeVMThread(Thread *thread);

    private:
        void initClassPath();
        void initOopManager();
        void initBootstrapClassLoader();
        void initStringPool();
        void initJavaSystemClass();
        void runMainMethod();
        void joinThreads();
    };

    void vmMain(ApplicationParameter &param);

}

#endif