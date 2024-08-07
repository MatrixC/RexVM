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
    struct VMThread;
    struct OopManager;

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

        std::mutex vmThreadMtx;
        std::deque<VMThread *> vmThreadDeque;

        explicit VM(ApplicationParameter &params);

        void start();

        void addStartThread(VMThread *vmThread);

        cstring javaHome{};
        cstring javaClassPath{};

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