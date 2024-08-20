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

    void VM::initVM() {
        //init JAVA_HOME
        javaHome = getJavaHome();
        if (javaHome.empty()) {
            cprintlnErr("Error: JAVA_HOME environment variable not found");
            std::exit(0);
        }

        //init basic class path
        classPath = CombineClassPath::getDefaultCombineClassPath(javaHome, params.userClassPath);
        javaClassPath = classPath->getVMClassPath();
        
        //init memory allocator
        oopManager = std::make_unique<OopManager>(*this);

        //init bootstrap classLoader
        bootstrapClassLoader = std::make_unique<ClassLoader>(*this, *classPath); 

        //init string pool
        stringPool = std::make_unique<StringPool>(*this, *bootstrapClassLoader);
        //String Pool初始化完成后才可以初始化全部基础类 否则有些类会因为没有String Pool初始化时报npe
        bootstrapClassLoader->initBasicJavaClass();

        //init gc collector
        garbageCollector = std::make_unique<GarbageCollect>(*this);

        //init Main Thread
        mainThread = std::unique_ptr<VMThread>(VMThread::createOriginVMThread(*this));
        mainThread->setName("main"); //like openjdk
        addStartThread(mainThread.get());
        garbageCollector->start();
    }

    void VM::runMainMethod() const {
        const auto userParams = params.userParams;

        const auto &className = getJVMClassName(userParams[0]);
        const auto runClass = bootstrapClassLoader->getInstanceClass(className);
        if (runClass == nullptr) {
            cprintlnErr("Error: Could not find or load main class {}", userParams[0]);
            std::exit(0);
        }
        const auto mainMethod = runClass->getMethod("main", "([Ljava/lang/String;)V", true);
        if (mainMethod == nullptr) {
            cprintlnErr("Error: Main method not found in class {}, please define the main method as:", userParams[0]);
            cprintlnErr("   public static void main(String[] args)");
            std::exit(0);
        }
        const auto mainMethodParmSize = userParams.size() - 1;
        const auto stringArray = oopManager->newStringObjArrayOop(mainThread.get(), mainMethodParmSize);

        if (userParams.size() > 1) {
            for (size_t i = 1; i < userParams.size(); ++i) {
                stringArray->data[i - 1] = stringPool->getInternString(mainThread.get(), userParams.at(i));
            }
        }

        const auto initMethod =
                bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SYSTEM)
                ->getMethod("initializeSystemClass", "()V", true);

        mainThread->addMethod(initMethod, {});
        mainThread->addMethod(mainMethod, {Slot(stringArray)});

        mainThread->start(nullptr, false);
        mainThread->join();
    }

    void VM::joinThreads() {
        while (true) {
            VMThread *vmThread;
            {
                std::lock_guard<SpinLock> lock(vmThreadLock);
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
        //all user thread exit
        exit = true;
        garbageCollector->join();
        stringPool->clear();
        garbageCollector->collectAll();
    }

    void VM::addStartThread(VMThread *thread) {
        std::lock_guard<SpinLock> lock(vmThreadLock);
        vmThreadDeque.emplace_back(thread);
    }

    bool VM::checkAllThreadStopForCollect() {
        std::lock_guard<SpinLock> lock(vmThreadLock);
        for (const auto &item : vmThreadDeque) {
            if (item->getStatus() != ThreadStatusEnum::TERMINATED && !item->stopForCollect) {
                return false;
            }
        }
        return true;
    }

    VM::VM(ApplicationParameter &params) : params(params) {
    }

    void VM::start() {
        initVM();
        runMainMethod();
        joinThreads();
        exitVM();
    }

    void vmMain(ApplicationParameter &param) {
        VM vm(param);
        vm.start();
    }

}