#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <deque>
#include <chrono>
#include "config.hpp"

namespace RexVM {

    struct ClassPath;
    struct ClassLoader;
    struct StringPool;
    struct NativeManager;
    struct VMThread;
    struct OopManager;
    struct Method;
    struct GarbageCollect;

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
        std::unique_ptr<GarbageCollect> garbageCollector;
        std::unique_ptr<VMThread> mainThread;

        std::mutex vmThreadMtx;
        std::deque<VMThread *> vmThreadDeque;
        bool exit{false};
        
        explicit VM(ApplicationParameter &params);

        void start();


        void addStartThread(VMThread *vmThread);
        bool checkAllThreadStopForCollect();

        void runStaticMethodOnMainThread(Method &method, std::vector<Slot> methodParams) const;

        std::chrono::system_clock::time_point startTime{std::chrono::system_clock::now()};
        cstring javaHome{};
        cstring javaClassPath{};

    private:
        void initClassPath();
        void initOopManager();
        void initBootstrapClassLoader();
        void initStringPool();
        void initMainThread();
        void initJavaSystemClass() const;
        void initCollector();
        void runMainMethod() const;
        void joinThreads();
        void exitVM();
    };

    void vmMain(ApplicationParameter &param);

}

#endif