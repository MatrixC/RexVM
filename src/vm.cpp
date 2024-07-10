#include "vm.hpp"
#include <thread>
#include <atomic>
#include "basic_java_class.hpp"
#include "utils/class_path.hpp"
#include "attribute_info.hpp"
#include "class_loader.hpp"
#include "constant_pool.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "execute.hpp"


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
        //String Pool初始化完成后才可以初始化全部基础类 否则有些类会因为没有String Pool初始化时报npe
        bootstrapClassLoader->initBasicJavaClass();
    }

    void VM::initJavaSystemClass() {
        const auto systemClass = bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SYSTEM);
        const auto initMethod = systemClass->getMethod("initializeSystemClass", "()V", true);
        runStaticMethodOnMainThread(*this, *initMethod, {});
    }

    void VM::runMainMethod() {
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
        const auto stringArrayClass = bootstrapClassLoader->getObjectArrayClass(JAVA_LANG_STRING_NAME);
        const auto mainMethodParmSize = userParams.size() - 1;
        const auto stringArray = oopManager->newObjArrayOop(stringArrayClass, mainMethodParmSize);

        if (userParams.size() > 1) {
            for (size_t i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = stringPool->getInternString(userParams.at(i));
            }
        }

        runStaticMethodOnMainThread(*this, *mainMethod, { Slot(stringArray) });
    }

    void VM::joinThreads() {
        while (true) {
            VMThread *vmThread;
            {
                std::lock_guard<std::mutex> lock(vmThreadMtx);
                if (vmThreadDeque.empty()) {
                    break;
                }
                vmThread = vmThreadDeque.front();
                vmThreadDeque.pop_front();
            }
            vmThread->join();
        }
    }

    void VM::addStartThread(VMThread *thread) {
        std::lock_guard<std::mutex> lock(vmThreadMtx);
        vmThreadDeque.emplace_back(thread);
    }

    VM::VM(ApplicationParameter &params) : params(params) {
    }

    void VM::start() {
        initClassPath();
        initOopManager();
        initBootstrapClassLoader();
        initStringPool();
        initJavaSystemClass();
        runMainMethod();
        joinThreads();
    }

    void vmMain(ApplicationParameter &param) {
        VM vm(param);
        vm.start();
        //gc2(vm);
        //vm.bootstrapClassLoader.reset(nullptr);

        // int x = 10;
        // bool s = compareAndSwapInt(&x, 11, 20);
        // cprintln("{} {}", s, x);
        



    }

}