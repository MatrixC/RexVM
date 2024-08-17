#include "vm.hpp"
#include <atomic>
#include <filesystem>
#include <utility>
#include "basic_java_class.hpp"
#include "utils/class_path.hpp"
#include "utils/class_utils.hpp"
#include "utils/string_utils.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "class_loader.hpp"
#include "string_pool.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "file_system.hpp"
#include "garbage_collect.hpp"


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

    void VM::initMainThread() {
        mainThread = std::make_unique<VMThread>(*this);

        const auto threadGroupClass = bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD_GROUP);
        const auto vmThreadGroup = oopManager->newInstance(mainThread.get(), threadGroupClass);
       
        mainThread->setFieldValue("group", "Ljava/lang/ThreadGroup;", Slot(vmThreadGroup));
        mainThread->setFieldValue("priority", "I", Slot(CAST_I8(1)));
        addStartThread(mainThread.get());

        garbageCollector->start();
    }

    void VM::runStaticMethodOnMainThread(Method &method, std::vector<Slot> methodParams) const {
        mainThread->reset(&method, std::move(methodParams));
        mainThread->start(nullptr);
        mainThread->join();
    }

    void VM::initJavaSystemClass() const {
        const auto systemClass = bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SYSTEM);
        const auto initMethod = systemClass->getMethod("initializeSystemClass", "()V", true);
        runStaticMethodOnMainThread(*initMethod, {});
    }

    void VM::initCollector() {
        garbageCollector = std::make_unique<GarbageCollect>(*this);
    }

    void VM::runMainMethod() const {
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
        const auto stringArray = oopManager->newStringObjArrayOop(mainThread.get(), mainMethodParmSize);

        if (userParams.size() > 1) {
            for (size_t i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = stringPool->getInternString(mainThread.get(), userParams.at(i));
            }
        }

        //runStaticMethodOnMainThread(*this, *mainMethod, {Slot(stringArray)});
        runStaticMethodOnMainThread(*mainMethod, {Slot(stringArray)});
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

    void VM::exitVM() {
        exit = true;
        garbageCollector->join();
        stringPool->clear();
        garbageCollector->collectAll();
    }

    void VM::addStartThread(VMThread *thread) {
        std::lock_guard<std::mutex> lock(vmThreadMtx);
        vmThreadDeque.emplace_back(thread);
    }

    bool VM::checkAllThreadStopForCollect() {
        std::lock_guard<std::mutex> lock(vmThreadMtx);
        for (const auto &item : vmThreadDeque) {
            if (!item->stopForCollect) {
                return false;
            }
        }
        return true;
    }

    VM::VM(ApplicationParameter &params) : params(params) {
    }

    void VM::start() {
        initClassPath();
        initCollector();
        initOopManager();
        initBootstrapClassLoader();
        initStringPool();
        initMainThread();
        initJavaSystemClass();
        runMainMethod();
        joinThreads();
        exitVM();
    }

    void vmMain(ApplicationParameter &param) {
        VM vm(param);
        vm.start();
    }

}