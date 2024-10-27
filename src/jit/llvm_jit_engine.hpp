#ifndef LLVM_JIT_ENGINE_HPP
#define LLVM_JIT_ENGINE_HPP
#include "../basic.hpp"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <atomic>

namespace RexVM {

    struct Method;
    struct VM;

    struct LLVM_JIT_Engine {
        VM &vm;
        std::atomic_uint32_t methodCnt{0};
        std::atomic_uint32_t successMethodCnt{0};
        std::atomic_uint32_t failedMethodCnt{0};

        explicit LLVM_JIT_Engine(VM &vm);
        ~LLVM_JIT_Engine();

        std::unique_ptr<llvm::orc::LLJIT> jit;
        std::unique_ptr<llvm::orc::ThreadSafeContext> threadSafeContext;

        void registerHelpFunction() const;

        CompiledMethodHandler compileMethod(Method &method);


    };

}

#endif
