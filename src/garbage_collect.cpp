#include "garbage_collect.hpp"
#include <unordered_set>
#include <algorithm>
#include "vm.hpp"
#include "thread.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "mirror_oop.hpp"
#include "class_loader.hpp"
#include "string_pool.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "os_platform.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"

namespace RexVM {

    GarbageCollectContext::GarbageCollectContext(VM &vm) {
        const auto &oopManager = vm.oopManager;
        allocatedOopCount = oopManager->allocatedOopCount;
        allocatedOopMemory = oopManager->allocatedOopMemory;
    }

    void GarbageCollectContext::endGetRoots() {
        getGcRootEndTime = std::chrono::system_clock::now();
    }

    void GarbageCollectContext::endTraceOop() {
        traceOopEndTime = std::chrono::system_clock::now();
    }

    void GarbageCollectContext::collectFinish(VM &vm) {
        endTime = std::chrono::system_clock::now();
        const auto &oopManager = vm.oopManager;
        oopManager->allocatedOopCount -= collectedOopCount;
        oopManager->allocatedOopMemory -= collectedOopMemory;
    }

    void GarbageCollectContext::printLog(VM &vm) const {
        const auto &oopManager = vm.oopManager;
        const auto timeCost = endTime - startTime;
        cprintln("gc [{:%H:%M:%S} cost:{}], crt:{}[{}KB], col:{}[{}KB], rem:{}[{}KB]",
                 startTime, timeCost,
                 allocatedOopCount, CAST_F4(allocatedOopMemory) / 1024,
                 collectedOopCount.load(), CAST_F4(collectedOopMemory) / 1024,
                 oopManager->allocatedOopCount.load(), CAST_F4(oopManager->allocatedOopMemory) / 1024
        );
    }

    GarbageCollect::GarbageCollect(VM &vm) : vm(vm), finalizeRunner(vm, *this) {
    }

    void GarbageCollect::start() {
        if (!enableGC) {
            return;
        }

        stringClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING);

        gcThread = std::thread([this]() {
            setThreadName("GC Thread");
            while (!this->vm.exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(collectSleepTime));
                if (vm.oopManager->allocatedOopMemory > collectMemoryThreshold) {
                    this->run();
                }
            }
            //finalize中有wait 防止因为wait而无法退出线程
            finalizeRunner.cv.notify_all();
        });

        //finalizeRunner.start();
    }

    void GarbageCollect::checkStopForCollect(VMThread &thread) {
        if (!markCollect) [[likely]] {
            return;
        }

        std::unique_lock<std::mutex> lock(mtx);
        thread.stopForCollect = true;
        cv.wait(lock, [this] { return !markCollect; });
        thread.stopForCollect = false;
    }

    bool GarbageCollect::checkTerminationCollect() {
        if (vm.exit) {
            //VM退出
            return false;
        }

        //如果有线程栈中有native函数 停止gc
        const auto threads = vm.threadManager->getThreads();
        for (const auto &thread: threads) {
            if (thread->hasNativeCall()) {
                return false;
            }
        }
        //可以继续gc
        return true;
    }

    bool GarbageCollect::stopTheWorld() {
        markCollect = true;
        finalizeRunner.cv.notify_all();
        const auto stopTime = getCurrentTimeMillis();
        while (!vm.threadManager->checkAllThreadStopForCollect() && !vm.exit) {
            if ((getCurrentTimeMillis() - stopTime) > collectStopWaitTimeout) {
                //stop wait timeout
                //return false;
            }
        }
        return true;
    }

    void GarbageCollect::startTheWorld() {
        markCollect = false;
        cv.notify_all();
    }

    void GarbageCollect::run() {
        const auto stopResult = stopTheWorld();
        if (!stopResult || !checkTerminationCollect()) {
            startTheWorld();
            return;
        }

        GarbageCollectContext context(vm);

        const auto gcRoots = getGarbageCollectRoots();
        context.endGetRoots();

        for (const auto &item: gcRoots) {
            traceMarkOop(item);
        }
        context.endTraceOop();

        collectOopHolder(vm.oopManager->defaultOopHolder, context);
        for (const auto &item: vm.threadManager->getThreads()) {
            collectOopHolder(item->oopHolder, context);
        }

        context.collectFinish(vm);
        sumCollectedMemory += context.allocatedOopMemory;
        if (enableLog) {
            context.printLog(vm);
        }

        //fuck!
        //or gcRoots all clearTraced()
        vm.mainThread->clearTraced();

        startTheWorld();
    }

    void GarbageCollect::deleteOop(ref oop) {
        const auto klass = oop->getClass();
#ifdef DEBUG
        const auto desc = cformat("{}:{}", klass->name, oop->isMirror());
        collectedOopDesc.emplace(oop, desc);
#endif
        if (klass->type == ClassTypeEnum::OBJ_ARRAY_CLASS) {
            delete CAST_OBJ_ARRAY_OOP(oop);
            return;
        }

        if (klass->type == ClassTypeEnum::TYPE_ARRAY_CLASS) {
            const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(klass);
            switch (typeArrayClass->elementType) {
                case BasicType::T_BOOLEAN:
                case BasicType::T_BYTE:
                    delete CAST_BYTE_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_SHORT:
                    delete CAST_SHORT_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_INT:
                    delete CAST_INT_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_LONG:
                    delete CAST_LONG_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_CHAR:
                    delete CAST_CHAR_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_FLOAT:
                    delete CAST_FLOAT_TYPE_ARRAY_OOP(oop);
                    return;
                case BasicType::T_DOUBLE:
                    delete CAST_DOUBLE_TYPE_ARRAY_OOP(oop);
                    return;
            }
        }

        if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
            const auto instanceClass = CAST_INSTANCE_CLASS(klass);
            const auto specialType = instanceClass->specialInstanceClass;
            if (oop->isMirror()) {
                delete CAST_MIRROR_OOP(oop);
                return;
            }
            if (specialType == SpecialInstanceClass::THREAD_CLASS) {
                delete CAST_VM_THREAD_OOP(oop);
                return;
            }
            delete CAST_INSTANCE_OOP(oop);
            return;
        }
    }

    void GarbageCollect::collectOopHolder(OopHolder &holder, GarbageCollectContext &context) {
        const auto &oops = holder.oops;
        std::vector<ref> survives;
        survives.reserve(oops.size() / 2);
        for (const auto &oop: oops) {
            if (oop->isTraced()) {
                survives.emplace_back(oop);
                oop->clearTraced();
            } else {
                if (!oop->isFinalized() && oop->getType() == OopTypeEnum::INSTANCE_OOP) [[unlikely]] {
                    //need to run finalize
                    survives.emplace_back(oop);
                    oop->clearTraced();
                    //add to run finalize
                    finalizeRunner.add(CAST_INSTANCE_OOP(oop));
                    continue;
                }

                const auto memorySize = oop->getMemorySize();

                ++context.collectedOopCount;
                context.collectedOopMemory += memorySize;

                const auto oopClass = oop->getClass();
                if (oopClass == stringClass) {
                    vm.stringPool->gcStringOop(CAST_INSTANCE_OOP(oop));
                }
                deleteOop(oop);
            }
        }
        holder.oops.swap(survives);
    }

    void GarbageCollect::getClassStaticRef(std::vector<ref> &gcRoots) const {
        auto &classLoader = *vm.bootstrapClassLoader;
        for (const auto &[name, klass]: classLoader.classMap) {
            const auto mirror = klass->getMirror(nullptr, false);
            if (mirror != nullptr) {
                gcRoots.emplace_back(mirror);
            }
            if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
                const auto instanceClass = CAST_INSTANCE_CLASS(klass.get());
                if (instanceClass->notInitialize()) {
                    continue;
                }

                for (const auto &field: instanceClass->fields) {
                    if (field->getFieldSlotType() == SlotTypeEnum::REF && field->isStatic()) {
                        if (const auto oop = instanceClass->getFieldValue(field->slotId).refVal; oop != nullptr) {
                            gcRoots.emplace_back(oop);
                        }
                    }
                }
            }
        }
    }

    void GarbageCollect::getThreadRef(std::vector<ref> &gcRoots) {
        //是否要考虑gc线程?
        for (const auto &thread: vm.threadManager->getThreads()) {
            const auto status = thread->getStatus();
            if (status != ThreadStatusEnum::TERMINATED) {
                thread->getThreadGCRoots(gcRoots);
                gcRoots.emplace_back(thread);
            } else {
                //TODO 考虑在这里回收Thread
            }
        }
    }

    std::vector<ref> GarbageCollect::getGarbageCollectRoots() {
        std::vector<ref> gcRoots;
        gcRoots.reserve(GC_ROOT_START_SIZE);
        getClassStaticRef(gcRoots);
        getThreadRef(gcRoots);
        return gcRoots;
    }

    void GarbageCollect::traceMarkOop(ref oop) const {
        if (oop == nullptr || oop->isTraced()) {
            return;
        }

        oop->markTraced();
        switch (oop->getType()) {
            case OopTypeEnum::INSTANCE_OOP:
                traceMarkInstanceOopChild(CAST_INSTANCE_OOP(oop));
                break;

            case OopTypeEnum::OBJ_ARRAY_OOP:
                traceMarkObjArrayOopChild(CAST_OBJ_ARRAY_OOP(oop));
                break;

            default:
                return;
        }
    }

    void GarbageCollect::traceMarkInstanceOopChild(InstanceOop *oop) const {
        const auto klass = oop->getInstanceClass();

        //要包含它父类的字段
        for (auto current = klass; current != nullptr; current = current->superClass) {
            for (const auto &field: current->fields) {
                const auto fieldType = field->getFieldSlotType();
                if (fieldType == SlotTypeEnum::REF && !field->isStatic()) {
                    const auto memberOop = oop->getFieldValue(field->slotId).refVal;
                    if (memberOop != nullptr) {
                        traceMarkOop(memberOop);
                    }
                }
            }
        }
    }

    void GarbageCollect::traceMarkObjArrayOopChild(ObjArrayOop *oop) const {
        const auto arrayLength = oop->getDataLength();
        if (arrayLength > 0) {
            FOR_FROM_ZERO(arrayLength) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceMarkOop(element);
                }
            }
        }
    }

    void GarbageCollect::collectAll() {
        cprintln("sumCollectedMemory:{}KB", CAST_F4(sumCollectedMemory) / 1024);

        std::vector<OopHolder *> oopHolders;
        oopHolders.emplace_back(&vm.oopManager->defaultOopHolder);
        for (const auto &item: vm.threadManager->getThreads()) {
            oopHolders.emplace_back(&item->oopHolder);
        }

        size_t deleteCount{0};

        std::vector<ref> lastCollect;
        for (const auto &item : oopHolders) {
            for (const auto &oop : item->oops) {
                const auto klass = oop->getClass();
                if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
                    const auto instanceClass = CAST_INSTANCE_CLASS(klass);
                    if (instanceClass->specialInstanceClass == SpecialInstanceClass::THREAD_CLASS) {
                        lastCollect.emplace_back(oop);
                        continue;
                    }
                }
                deleteOop(oop);
                ++deleteCount;
            }
        }

        for (const auto &item : lastCollect) {
            deleteOop(item);
            ++deleteCount;
        }

        cprintln("{}, {}", vm.oopManager->allocatedOopCount.load(), deleteCount);
    }

    void GarbageCollect::join() {
        finalizeRunner.cv.notify_all();

        if (gcThread.joinable()) {
            gcThread.join();
        }
    }

    FinalizeRunner::FinalizeRunner(VM &vm, GarbageCollect &collector) : collector(collector)  {
    }

    void FinalizeRunner::add(InstanceOop *oop) {
        {
            std::lock_guard<std::mutex> lock(dequeMtx);
            oopDeque.emplace_back(oop);
        }
        cv.notify_all();
    }

    void FinalizeRunner::runOopFinalize(InstanceOop *oop) const {
        const auto klass = oop->getInstanceClass();
        const auto finalizeMethod = klass->getMethod("finalize", "()V", false);
        if (finalizeMethod == nullptr) [[unlikely]] {
            cprintlnErr("run finalize error: get finalizeMethod fail");
        } else {
            createFrameAndRunMethod(*finalizeThread, *finalizeMethod, {Slot(oop)}, nullptr);
        }
        oop->setFinalized(true);
    }

    bool FinalizeRunner::runOneOop() {
        InstanceOop *oop = nullptr;
        {
            std::unique_lock<std::mutex> lock(dequeMtx);
            if (oopDeque.empty()) {
                return false;
            }
            oop = oopDeque.front();
            oopDeque.pop_front();
        }

        if (oop != nullptr) {
            runOopFinalize(oop);
            return true;
        }
        return false;
    }

    void FinalizeRunner::runnerMethod() {
        while (!collector.vm.exit) {
            const auto runOne = runOneOop();
            collector.checkStopForCollect(*finalizeThread);
            
            if (!runOne) {
                std::unique_lock<std::mutex> lock(dequeMtx);
                cv.wait(lock, [this] { 
                    return !oopDeque.empty() 
                            || collector.markCollect
                            || collector.vm.exit; 
                });
            }
        }
    }

    void FinalizeRunner::initFinalizeThread(VMThread *mainThread) {
        const auto &vm = collector.vm;
        const auto threadClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        finalizeThread = CAST_VM_THREAD_OOP(vm.oopManager->newInstance(mainThread, threadClass));
        std::function<void()> func = std::bind(&FinalizeRunner::runnerMethod, this);
        finalizeThread->addMethod(func);

        const auto threadConstructor = threadClass->getMethod("<init>", "(Ljava/lang/String;)V", false);
        const auto setDaemonMethod = threadClass->getMethod("setDaemon", "(Z)V", false);
        const auto threadNameOop = vm.stringPool->getInternString(mainThread, "Finalize Thread");

        std::vector<Slot> constructorParams = { 
            Slot(finalizeThread), 
            Slot(threadNameOop) 
        };

        std::vector<Slot> setDaemonParams = { 
            Slot(finalizeThread), 
            Slot(CAST_I4(1))
        };
        
        createFrameAndRunMethod(*finalizeThread, *threadConstructor, constructorParams, nullptr, true);
        createFrameAndRunMethod(*finalizeThread, *setDaemonMethod, setDaemonParams, nullptr, true);
        finalizeThread->start(nullptr, false);

    }


#ifdef DEBUG
    std::unordered_map<ref, cstring> collectedOopDesc;
    cstring getCollectedOopDesc(ref oop) {
        const auto iter = collectedOopDesc.find(oop);
        if (iter != collectedOopDesc.end()) {
            return iter->second;
        }
        return "not found";
    }
#endif 

}