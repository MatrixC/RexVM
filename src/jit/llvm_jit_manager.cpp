#include "llvm_jit_manager.hpp"
#include <memory>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include "../vm.hpp"
#include "../class_member.hpp"
#include "llvm_compiler.hpp"
#include "jit_help_function.hpp"
#include "../class.hpp"
#include "../utils/string_utils.hpp"

#define DEFINE_SYMBOL(hfname) symbol_map[mangle(#hfname)] = ExecutorSymbolDef(ExecutorAddr::fromPtr(&hfname), JITSymbolFlags());


namespace RexVM {
    using namespace llvm;
    using namespace llvm::orc;

    LLVM_JITManager::LLVM_JITManager(VM &vm) : JITManager(vm) {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        jit = cantFail(LLJITBuilder().create());
        threadSafeContext = std::make_unique<ThreadSafeContext>(std::make_unique<LLVMContext>());
        registerHelpFunction();
    }

    void LLVM_JITManager::registerHelpFunction() const {
        auto &jd = jit->getMainJITDylib();
        const auto &dataLayout = jit->getDataLayout();
        auto &executionSession = jd.getExecutionSession();
        auto mangle = MangleAndInterner(executionSession, dataLayout);

        SymbolMap symbol_map;

        DEFINE_SYMBOL(llvm_compile_get_instance_constant)
        DEFINE_SYMBOL(llvm_compile_array_length)
        DEFINE_SYMBOL(llvm_compile_return_common)
        DEFINE_SYMBOL(llvm_compile_clinit)
        DEFINE_SYMBOL(llvm_compile_invoke_method_fixed)
        DEFINE_SYMBOL(llvm_compile_new_object)
        DEFINE_SYMBOL(llvm_compile_throw_exception)
        DEFINE_SYMBOL(llvm_compile_check_cast)
        DEFINE_SYMBOL(llvm_compile_monitor)

        cantFail(jd.define(absoluteSymbols(symbol_map)));
    }


    CompiledMethodHandler LLVM_JITManager::compileMethod(Method &method) {
        if (!method.exceptionCatches.empty()) {
            return nullptr;
        }
        const auto ctx = threadSafeContext->getContext();
        const auto currentMethodCnt = methodCnt.fetch_add(1);
        const auto moduleName = cformat("module_{}", currentMethodCnt);
        const auto compiledMethodName = cformat("{}_{}", method.getName(), currentMethodCnt);
        auto module = std::make_unique<Module>(moduleName, *ctx);

        MethodCompiler methodCompiler(vm, method, *module, compiledMethodName);
        if (!methodCompiler.compile()) {
            return nullptr;;
        }
        methodCompiler.verify();
        // cprintln("compiled method {} {}", method.klass.getClassName(), method.getName());

        // if (startWith(compiledMethodName, "saveAndRemoveProperties")) {
             module->print(llvm::outs(), nullptr);
        // }

        auto TSM = ThreadSafeModule(std::move(module), *threadSafeContext);
        cantFail(jit->addIRModule(std::move(TSM)));

        const auto sym = jit->lookup(compiledMethodName);
        const auto ptr = sym->toPtr<CompiledMethodHandler>();
        method.compiledMethodHandler = ptr;
        return ptr;
    }
}
