#ifndef VM_HPP
#define VM_HPP

#include <memory>
#include <vector>
#include "config.hpp"

namespace RexVM {

    struct ClassPath;
    struct ClassLoader;
    struct StringPool;
    struct Executor;
    struct NativeManager;
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
        std::unique_ptr<Executor> executor;
        std::unique_ptr<ClassLoader> bootstrapClassLoader;
        explicit VM(ApplicationParameter &params);

    private:
        void initClassPath();
        void initOopManager();
        void initBootstrapClassLoader();
        void initStringPool();
        void initExecutor();
        void initJavaSystemClass() const;
        void runMainMethod() const;

    };

    void vmMain(ApplicationParameter &param);

}

#endif