#ifndef LLVM_JIT_MANAGER_HPP
#define LLVM_JIT_MANAGER_HPP
#include <memory>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <atomic>
#include "jit_manager.hpp"

namespace RexVM {

    struct Method;

    class LLVMJITManager : public JITManager {
    public:
        std::atomic_int32_t methodCnt{0};

        explicit LLVMJITManager(VM &vm);

        std::unique_ptr<llvm::orc::LLJIT> jit;
        std::unique_ptr<llvm::orc::ThreadSafeContext> threadSafeContext;

        void compileMethod(Method &method) override;

    };

}

#endif
