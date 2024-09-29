#include "llvm_jit_manager.hpp"
#include <memory>
#include "../vm.hpp"
#include "../class_member.hpp"
#include "llvm_compiler.hpp"
#include <llvm/Support/TargetSelect.h>


namespace RexVM {
    using namespace llvm;
    using namespace llvm::orc;

    LLVMJITManager::LLVMJITManager(VM &vm) : JITManager(vm) {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        jit = cantFail(LLJITBuilder().create());
        threadSafeContext = std::make_unique<ThreadSafeContext>(std::make_unique<LLVMContext>());
    }

    void LLVMJITManager::compileMethod(Method &method) {
        const auto ctx = threadSafeContext->getContext();
        const auto currentMethodCnt = methodCnt.fetch_add(1);
        const auto moduleName = cformat("module_{}",currentMethodCnt);
        const auto compiledMethodName = cformat("{}_{}", method.getName(), currentMethodCnt);
        auto module = std::make_unique<Module>(moduleName, *ctx);

        MethodCompiler methodCompiler(method, *module, compiledMethodName);
        methodCompiler.compile();

        module->print(llvm::outs(), nullptr);

        auto TSM = ThreadSafeModule(std::move(module), *threadSafeContext);
        cantFail(jit->addIRModule(std::move(TSM)));

        const auto sym = jit->lookup(compiledMethodName);
        const auto ptr = sym->toPtr<CompiledMethodHandler>();
        method.compiledMethodHandler = ptr;
    }

}
