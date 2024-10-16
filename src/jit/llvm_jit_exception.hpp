#ifndef LLVM_JIT_EXCEPTION_HPP
#define LLVM_JIT_EXCEPTION_HPP
#include "../config.hpp"

namespace RexVM {

    struct MethodCompiler;

    struct MethodExceptionCompiler {

        MethodCompiler &methodCompiler;

        explicit MethodExceptionCompiler(MethodCompiler &methodCompiler);

        void parseExceptionTable();


    };
}

#endif
