#ifndef JIT_MANAGER_HPP
#define JIT_MANAGER_HPP

namespace RexVM {

    struct VM;
    struct Method;

    class JITManager {
    public:
        explicit JITManager(VM &vm) {
        }
        virtual ~JITManager() = default;

        virtual void compileMethod(Method &method) = 0;
    };

}

#endif