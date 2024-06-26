#include "vm.hpp"
#include "utils/class_path.hpp"
#include "constantInfo.hpp"
#include "attribute_info.hpp"
#include "class_loader.hpp"
#include "constant_pool.hpp"
#include "class.hpp"
#include "runtime.hpp"
#include "frame.hpp"
#include "oop.hpp"
#include "memory.hpp"
#include "native/native_manager.hpp"

#include "utils/format.hpp"
#include "class_file.hpp"
#include "class_file_print.hpp"
#include "utils/descriptor_parser.hpp"
#include "memory.hpp"
#include <ranges>


namespace RexVM {

    constexpr auto ENV_KEY_JAVA_HOME = "JAVA8_HOME";
    constexpr auto USER_PROGRAM_HOME = "USER_JAVA_CLASSPATH";

    void VM::initClassPath() {
        const auto javaHome = cstring(std::getenv(ENV_KEY_JAVA_HOME));
        const auto rtJarPath = javaHome + "/jre/lib/rt.jar";
        const auto charsetsJarPath = javaHome + "/jre/lib/charsets.jar";
        const auto userClassPath = cstring(std::getenv(USER_PROGRAM_HOME));
        classPath = std::make_unique<CombineClassPath>(rtJarPath + ";" + charsetsJarPath + ";" + userClassPath);
    }

    void VM::initOopManager() {
        oopManager = std::make_unique<OopManager>(*this);
    }

    void VM::initBootstrapClassLoader() {
        bootstrapClassLoader = std::make_unique<ClassLoader>(*this, *classPath);
    }

    void VM::initStringPool() {
        stringPool = std::make_unique<StringPool>(*this, *bootstrapClassLoader);
    }

    void VM::initExecutor() {
        executor = std::make_unique<Executor>(*this);
    }

    void VM::initJavaSystemClass() const {
        const auto systemClass = bootstrapClassLoader->getInstanceClass("java/lang/System");
        const auto initMethod = systemClass->getMethod("initializeSystemClass", "()V", true);
        executor->runStaticMethodOnNewThread(*initMethod, {});
    }

    void VM::runMainMethod() const {
        const auto userParams = params.userParams;

        if (userParams.empty()) {
            panic("must input run class");
        }

        const auto &className = userParams.at(0);
        const auto runClass = bootstrapClassLoader->getInstanceClass(className);
        if (runClass == nullptr) {
            panic("can't find class " + className);
        }
        const auto mainMethod = runClass->getMethod("main", "([Ljava/lang/String;)V", true);
        if (mainMethod == nullptr) {
            panic("no main method in class " + className);
        }
        const auto stringArrayClass = bootstrapClassLoader->getObjectArrayClass("java/lang/String");
        const auto mainMethodParmSize = userParams.size() - 1;
        const auto stringArray = oopManager->newObjArrayOop(stringArrayClass, mainMethodParmSize);

        if (userParams.size() > 1) {
            for (auto i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = stringPool->getInternString(userParams.at(i));
            }
        }

        executor->runStaticMethodOnNewThread(*mainMethod, std::vector{ Slot(stringArray) });
    }



    VM::VM(ApplicationParameter &params) : params(params) {
        initClassPath();
        initOopManager();
        initBootstrapClassLoader();
        initStringPool();
        initExecutor();
        initJavaSystemClass();
        runMainMethod();
    }

  
    void vmMain(ApplicationParameter &param) {
        VM vm(param);
        vm.bootstrapClassLoader.reset(nullptr);
        gc2(vm);
    }

}