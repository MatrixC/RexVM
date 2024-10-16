#include "llvm_jit_exception.hpp"
#include <llvm/IR/IRBuilder.h>
#include "llvm_compiler.hpp"
#include "../class_member.hpp"

namespace RexVM {

    using namespace llvm;

    MethodExceptionCompiler::MethodExceptionCompiler(MethodCompiler &methodCompiler)
        : methodCompiler(methodCompiler) {
    }

    void MethodExceptionCompiler::parseExceptionTable() {
        const auto &method = methodCompiler.method;
        if (method.exceptionCatches.empty()) {
            return;
        }

        llvm::Value *pc{nullptr};


        for (const auto &exception : method.exceptionCatches) {



        }
    }
}
