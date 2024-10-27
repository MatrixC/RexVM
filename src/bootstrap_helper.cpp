#include "bootstrap_helper.hpp"
#include <functional>
#include <vector>
#include "vm.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "string_pool.hpp"
#include "garbage_collect.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    bool initInitializeMethod(VM &vm) {
        const auto mainThread = vm.mainThread.get();
        const auto initializeSystemClassMethod =
            vm.bootstrapClassLoader
                ->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SYSTEM)
                ->getMethod("initializeSystemClass" "()V", true);
        if (initializeSystemClassMethod == nullptr) {
            cprintlnErr("Error: Could not find or load system class {}", JAVA_LANG_SYSTEM_NAME);
            return false;
        }

        std::function<void()> initFinalizeThread = [&vm]() {
            vm.garbageCollector->finalizeRunner.initFinalizeThread(vm.mainThread.get());
        };

        //0是 initializeSystemClass
        //1是 finalize
        //2是 main

        mainThread->addMethod(initializeSystemClassMethod, {});
        mainThread->addMethod(initFinalizeThread);
        return true;
    }

    bool initVMMainMethod(const VM &vm) {
        const auto &userParams = vm.params.userParams;
        const auto mainThread = vm.mainThread.get();

        const auto &className = getJVMClassName(userParams[0]);
        const auto runClass = vm.bootstrapClassLoader->getInstanceClass(className);
        if (runClass == nullptr) {
            cprintlnErr("Error: Could not find or load main class {}", userParams[0]);
            return false;
        }
        const auto mainMethod = runClass->getMethod("main" "([Ljava/lang/String;)V", true);
        if (mainMethod == nullptr) {
            cprintlnErr("Error: Main method not found in class {}, please define the main method as:", userParams[0]);
            cprintlnErr("   public static void main(String[] args)");
            return false;
        }
        const auto mainMethodParmSize = userParams.size() - 1;
        const auto stringArray = vm.oopManager->newStringObjArrayOop(mainThread, mainMethodParmSize);

        if (userParams.size() > 1) {
            for (size_t i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = vm.stringPool->getInternString(mainThread, userParams[i]);
            }
        }

        const auto initMethod = runClass->getMethod("<clinit>", "()V", true);
        if (initMethod != nullptr) {
            mainThread->addMethod(initMethod, {});
        }

        mainThread->addMethod(mainMethod, {Slot(stringArray)});
        return true;
    }

    bool initVMBootstrapMethods(VM &vm) {
        if (!initInitializeMethod(vm)) {
            return false;
        }
        if (!initVMMainMethod(vm)) {
            return false;
        }
        return true;
    }

}