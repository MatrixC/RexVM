#ifndef JIT_MANAGER_HPP
#define JIT_MANAGER_HPP
#include "../config.hpp"

namespace RexVM {

    struct VM;
    struct Method;

    class JITManager {
    public:
        explicit JITManager(VM &vm) : vm(vm) {
        }

        virtual ~JITManager() = default;

        VM &vm;

        virtual CompiledMethodHandler compileMethod(Method &method) = 0;
    };

}

#endif
