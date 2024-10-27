#ifndef JIT_MANAGER_HPP
#define JIT_MANAGER_HPP
#include "basic.hpp"
#include <thread>
#include <queue>
#include "utils/spin_lock.hpp"

namespace RexVM {

    struct VM;
    struct Method;
    struct Frame;
    struct LLVM_JIT_Engine;

    struct JITManager {

#ifdef LLVM_JIT
        VM &vm;
        size_t compileThreadSleepTime{200};
        size_t compileThreadPopCount{100};
        SpinLock queueLock;
        std::queue<Method *> compileMethods;
        std::unique_ptr<LLVM_JIT_Engine> llvmEngine;
        std::thread compileThread;
#endif

        explicit JITManager(VM &vm);

        ~JITManager();

        void checkCompile(Method &method);

        CompiledMethodHandler compileMethod(Method &method);
    };

}

#endif
