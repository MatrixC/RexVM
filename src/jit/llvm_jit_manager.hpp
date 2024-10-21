#ifndef LLVM_JIT_MANAGER_HPP
#define LLVM_JIT_MANAGER_HPP
#include "../config.hpp"
#include "jit_manager.hpp"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <atomic>

namespace RexVM {

    struct Method;

    class LLVM_JITManager final : public JITManager {
    public:
        std::atomic_uint32_t methodCnt{0};
        std::atomic_uint32_t successMethodCnt{0};
        std::atomic_uint32_t failedMethodCnt{0};

        explicit LLVM_JITManager(VM &vm);
        ~LLVM_JITManager() override;

        std::unique_ptr<llvm::orc::LLJIT> jit;
        std::unique_ptr<llvm::orc::ThreadSafeContext> threadSafeContext;

        void registerHelpFunction() const;

        CompiledMethodHandler compileMethod(Method &method) override;


    };

}

#endif
