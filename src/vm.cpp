#include "vm.hpp"
#include <atomic>
#include <filesystem>
#include "basic_java_class.hpp"
#include "utils/class_path.hpp"
#include "utils/class_utils.hpp"
#include "utils/string_utils.hpp"
#include "class_loader.hpp"
#include "string_pool.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "file_system.hpp"


namespace RexVM {

    void VM::initClassPath() {
        javaHome = initJavaHome(std::getenv("JAVA_HOME"));
        if (javaHome.empty()) {
            javaHome = initJavaHome(std::getenv("JAVA8_HOME"));
            if (javaHome.empty()) {
                cprintlnErr("Error: JAVA_HOME environment variable not found");
                std::exit(1);
            }
        }
        std::vector<cstring> pathList;
        pathList.emplace_back(".");
        pathList.emplace_back(buildRtPath(javaHome));
        pathList.emplace_back(buildCharsetsPath(javaHome));
        const auto userEnvClassPath = std::getenv("CLASSPATH");
        if (userEnvClassPath != nullptr) {
            const auto envClassPath = cstring(userEnvClassPath);
            const auto cps = splitString(envClassPath, PATH_SEPARATOR);
            for (const auto &item: cps) {
                pathList.emplace_back(item);
            }
        }
        if (!params.userClassPath.empty()) {
            const auto cps = splitString(params.userClassPath, PATH_SEPARATOR);
            for (const auto &item: cps) {
                pathList.emplace_back(item);
            }
        }

        classPath = std::make_unique<CombineClassPath>(joinString(pathList, cstring{PATH_SEPARATOR}));
        javaClassPath = classPath->getVMClassPath();
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

        const auto &className = getJVMClassName(userParams[0]);
        const auto runClass = bootstrapClassLoader->getInstanceClass(className);
        if (runClass == nullptr) {
            cprintlnErr("Error: Could not find or load main class {}", userParams[0]);
            std::exit(1);
        }
        const auto mainMethod = runClass->getMethod("main", "([Ljava/lang/String;)V", true);
        if (mainMethod == nullptr) {
            cprintlnErr("Error: Main method not found in class {}, please define the main method as:", userParams[0]);
            cprintlnErr("   public static void main(String[] args)");
            std::exit(1);
        }
        const auto mainMethodParmSize = userParams.size() - 1;
        const auto stringArray = oopManager->newStringObjArrayOop(mainMethodParmSize);

        if (userParams.size() > 1) {
            for (size_t i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = stringPool->getInternString(userParams.at(i));
            }
        }

        runStaticMethodOnMainThread(*this, *mainMethod, {Slot(stringArray)});
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
        collectAll(vm);
    }

}