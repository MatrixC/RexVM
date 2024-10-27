#include "jit_manager.hpp"
#include "vm.hpp"
#include "class_member.hpp"
#ifdef LLVM_JIT
#include "jit/llvm_jit_engine.hpp"
#endif

namespace RexVM {
#ifdef LLVM_JIT
    JITManager::JITManager(VM &vm) : vm(vm), llvmEngine(std::make_unique<LLVM_JIT_Engine>(vm)) {
        compileThread = std::thread([this]() {
            while (!this->vm.exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(compileThreadSleepTime));
                std::vector<Method *> stayCompileMethods; {
                    std::lock_guard guard(queueLock);
                    for (size_t i = 0; i < compileThreadPopCount; ++i) {
                        if (compileMethods.empty()) {
                            break;
                        }
                        stayCompileMethods.emplace_back(compileMethods.front());
                        compileMethods.pop();
                    }
                }
                if (!stayCompileMethods.empty()) {
                    for (const auto &stayCompileMethod: stayCompileMethods) {
                        if (this->vm.exit) {
                            return;
                        }
                        if (!stayCompileMethod->canCompile
                            || stayCompileMethod->isNative()
                            || stayCompileMethod->compiledMethodHandler != nullptr) {
                            continue;
                        }
                        llvmEngine->compileMethod(*stayCompileMethod);
                    }
                }
            }
        });
    }

    void JITManager::checkCompile(Method &method) {
        if (!vm.params.jitEnable
            || !method.canCompile
            || method.markCompile
            || method.isNative()
            || method.compiledMethodHandler != nullptr
            || method.invokeCounter < vm.params.jitCompileMethodInvokeCountThreshold) {
            return;
        }
        method.markCompile = true;
        std::lock_guard guard(queueLock);
        compileMethods.emplace(&method);
    }

    JITManager::~JITManager() {
        if (compileThread.joinable()) {
            compileThread.join();
        }
    }
#else
    JITManager::JITManager(VM &vm) {}
    void JITManager::checkCompile(Method &method) {};
    JITManager::~JITManager() = default;
#endif
}
