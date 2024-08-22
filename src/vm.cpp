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
#include "bootstrap_helper.hpp"


namespace RexVM {

    bool VM::initVM() {
        //init JAVA_HOME
        javaHome = getJavaHome();
        if (javaHome.empty()) {
            cprintlnErr("Error: JAVA_HOME environment variable not found");
            return false;
        }

        //init basic class path
        classPath = CombineClassPath::getDefaultCombineClassPath(javaHome, params.userClassPath);
        javaClassPath = classPath->getVMClassPath();
        
        //init memory allocator
        oopManager = std::make_unique<OopManager>(*this);

        //init thread manager
        threadManager = std::make_unique<ThreadManager>(*this);

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
        garbageCollector->start();

        if (!initVMBootstrapMethods(*this)) {
            return false;
        }

        return true;
    }

    void VM::runMainMethod() const {
        mainThread->start(nullptr, true);
    }

    void VM::joinThreads() {
        threadManager->joinUserThreads();

        //all user thread exit
        exit = true;
    }

    void VM::exitVM() {
        garbageCollector->join();
        stringPool->clear();
        garbageCollector->collectAll();
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