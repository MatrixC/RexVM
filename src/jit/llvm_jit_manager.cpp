#include "llvm_jit_manager.hpp"
#include <memory>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include "../vm.hpp"
#include "../class_member.hpp"
#include "llvm_compiler.hpp"
#include "jit_help_function.hpp"

#define DEFINE_SYMBOL(hfname) symbol_map[mangle("hfname")] = ExecutorSymbolDef(ExecutorAddr::fromPtr(&hfname), JITSymbolFlags());


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
        symbol_map[mangle("llvm_compile_get_string_constant")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_get_string_constant), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_get_class_mirror_constant")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_get_class_mirror_constant),
                                          JITSymbolFlags());

        symbol_map[mangle("llvm_compile_throw_npe")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_throw_npe), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_load_i4")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_load_i4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_load_i8")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_load_i8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_load_f4")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_load_f4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_load_f8")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_load_f8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_load_obj")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_load_obj), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_store_i4")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_store_i4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_store_i8")] =
                        ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_store_i8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_store_f4")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_store_f4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_store_f8")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_store_f8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_array_store_obj")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_array_store_obj), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_i4")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_i4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_i8")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_i8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_f4")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_f4), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_f8")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_f8), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_obj")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_obj), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_return_void")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_return_void), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_clinit")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_clinit), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_get_field")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_get_field), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_invoke_method")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_invoke_method), JITSymbolFlags());

        symbol_map[mangle("llvm_compile_invoke_method_fixed")] =
                ExecutorSymbolDef(ExecutorAddr::fromPtr(&llvm_compile_invoke_method_fixed), JITSymbolFlags());


       cantFail(jd.define(absoluteSymbols(symbol_map)));
    }


    void LLVM_JITManager::compileMethod(Method &method) {
        const auto ctx = threadSafeContext->getContext();
        const auto currentMethodCnt = methodCnt.fetch_add(1);
        const auto moduleName = cformat("module_{}",currentMethodCnt);
        const auto compiledMethodName = cformat("{}_{}", method.getName(), currentMethodCnt);
        auto module = std::make_unique<Module>(moduleName, *ctx);

        MethodCompiler methodCompiler(vm, method, *module, compiledMethodName);
        methodCompiler.compile();

        module->print(llvm::outs(), nullptr);

        auto TSM = ThreadSafeModule(std::move(module), *threadSafeContext);
        cantFail(jit->addIRModule(std::move(TSM)));

        const auto sym = jit->lookup(compiledMethodName);
        const auto ptr = sym->toPtr<CompiledMethodHandler>();
        method.compiledMethodHandler = ptr;
    }

}
