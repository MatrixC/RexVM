#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
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
        std::vector<std::unique_ptr<Thread>> threads;
        explicit VM(ApplicationParameter &params);

    private:
        void initClassPath();
        void initOopManager();
        void initBootstrapClassLoader();
        void initStringPool();
        void initJavaSystemClass();
        void runMainMethod();

    };

    void vmMain(ApplicationParameter &param);

}

#endif