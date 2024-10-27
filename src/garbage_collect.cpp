#include "garbage_collect.hpp"
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

    GarbageCollectContext::GarbageCollectContext(const VM &vm) : startTime(getCurrentTimeMillis()) {
        const auto &oopManager = vm.oopManager;
        tempAllocatedOopCount = oopManager->allocatedOopCount;
        tempAllocatedOopMemory = oopManager->allocatedOopMemory;
    }

    void GarbageCollectContext::endGetRoots() {
        getGcRootEndTime = getCurrentTimeMillis();
    }

    void GarbageCollectContext::endTraceOop() {
        traceOopEndTime = getCurrentTimeMillis();
    }

    void GarbageCollectContext::collectFinish(const VM &vm) {
        endTime = getCurrentTimeMillis();
        const auto &oopManager = vm.oopManager;
        oopManager->allocatedOopCount -= collectedOopCount;
        oopManager->allocatedOopMemory -= collectedOopMemory;
    }

    void GarbageCollectContext::printLog(const VM &vm) const {
        const auto &oopManager = vm.oopManager;
        const auto timeCost = endTime - startTime;
        cprintln("gc [{} {}ms], crt:{}[{:.2f}KB], col:{}[{:.2f}KB], rem:{}[{:.2f}KB]",
                 millisecondsToReadableTime(startTime), timeCost,
                 tempAllocatedOopCount, CAST_F4(tempAllocatedOopMemory) / 1024,
                 collectedOopCount.load(), CAST_F4(collectedOopMemory) / 1024,
                 oopManager->allocatedOopCount.load(), CAST_F4(oopManager->allocatedOopMemory) / 1024
        );
    }

    GarbageCollect::GarbageCollect(VM &vm)
        : vm(vm),
          finalizeRunner(vm, *this),
          gcRootReserveSize(vm.params.gcGCRootReserveSize),
          collectMemoryThreshold(vm.params.gcCollectMemoryThreshold),
          collectStopWaitTimeout(vm.params.gcCollectStopWaitTimeout),
          collectSleepTime(vm.params.gcCollectSleepTime),
          enableGC(vm.params.gcEnable),
          enableLog(vm.params.gcEnableLog),
          enableFinalize(vm.params.gcEnableFinalize) {
    }

    void GarbageCollect::notify() {
        if (!enableGC) {
            return;
        }

        notifyCollect = true;
        notifyCollectCv.notify_all();
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
    }

    void GarbageCollect::checkStopForCollect(VMThread &thread) {
        if (!enableGC) {
            return;
        }

        if (!checkStop) [[likely]] {
            return;
        }

        std::unique_lock lock(checkStopMtx);
        thread.stopForCollect = true;
        checkStopCv.wait(lock, [this] { return !checkStop; });
        thread.stopForCollect = false;
    }

    bool GarbageCollect::checkTerminationCollect() const {
        if (vm.exit) {
            //VM退出
            return false;
        }

        //如果有线程栈中有native函数 停止gc
        for (const auto threads = vm.threadManager->getThreads();
            const auto &thread: threads) {
            if (!thread->isGCSafe()) {
                //线程内有GC不安全的代码区块ni
                return false;
            }
        }
        //可以继续gc
        return true;
    }

    bool GarbageCollect::stopTheWorld() {
        checkStop = true;
        finalizeRunner.cv.notify_all();
        const auto stopTime = getCurrentTimeMillis();
        while (!vm.threadManager->checkAllThreadStopForCollect() && !vm.exit) {
            if ((getCurrentTimeMillis() - stopTime) > CAST_I8(collectStopWaitTimeout)) {
                //stop wait timeout
                //GC Thread跟UserThread可能出现互相死锁的情况 假设
                // 1. GC Thread发起StopTheWorld
                // 2. main Thread启动了两个其他的thread并且join他们 但他们被checkStop了
                // 所以join无法运行过去，main线程走不到checkStop位置 导致死锁
                return false;
            }
        }
        return true;
    }

    void GarbageCollect::startTheWorld() {
        checkStop = false;
        checkStopCv.notify_all();
    }

    void GarbageCollect::process() {
        GarbageCollectContext context(vm);
        processTrace(context);
        processCollect(context);

        sumCollectedMemory += context.tempAllocatedOopMemory;
        if (enableLog) {
            context.printLog(vm);
        }

        vm.mainThread->clearTraced();
        ++collectSuccessCount;
    }

    void GarbageCollect::run() {
        ++collectStartCount;
        const auto stopResult = stopTheWorld();
        if (!stopResult || !checkTerminationCollect()) {
            startTheWorld();
            return;
        }

        process();

        startTheWorld();
    }

    void GarbageCollect::deleteOop(ref oop) const {
        const auto klass = oop->getClass();

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
                
                default:
                    panic("error type");
            }
        }

        if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
            const auto instanceClass = CAST_INSTANCE_CLASS(klass);
            if (oop->isMirror()) {
                delete CAST_MIRROR_OOP(oop);
                return;
            }
            if (instanceClass->specialClassType == SpecialClassEnum::THREAD_CLASS) {
                delete CAST_VM_THREAD_OOP(oop);
                return;
            }
            delete CAST_INSTANCE_OOP(oop);
            return;
        }

        panic("error");
    }

    void GarbageCollect::processCollect(GarbageCollectContext &context) const {
        for (const auto holders = getHolders();
            const auto &holder : holders) {
            collectOopHolder(*holder, context);
        }
        context.collectFinish(vm);
    }

    void GarbageCollect::collectOopHolder(OopHolder &holder, GarbageCollectContext &context) const {
        const auto &oops = holder.oops;
        std::vector<ref> survives;
        survives.reserve(oops.size() / 2);
        for (const auto &oop: oops) {
            if (oop->isTraced()) {
                survives.emplace_back(oop);
                oop->clearTraced();
            } else {
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
                            //java/lang/invoke/SimpleMethodHandle.SPECIES_DATA 在check模式下报错
                            //oop地址为0xbebebebe00000000
                            gcRoots.emplace_back(oop);
                        }
                    }
                }
            }
        }
    }

    void GarbageCollect::getThreadRef(std::vector<ref> &gcRoots) const {
        //是否要考虑gc线程?
        for (const auto &thread: vm.threadManager->getThreads()) {
            const auto status = thread->getStatus();
            if (status != ThreadStatusEnum::TERMINATED) {
                thread->getCollectRoots(gcRoots);
                gcRoots.emplace_back(thread);
            } else {
                //TODO 考虑在这里回收Thread
            }
        }
    }

    std::vector<ref> GarbageCollect::getGarbageCollectRoots() const {
        std::vector<ref> gcRoots;
        gcRoots.reserve(vm.params.gcGCRootReserveSize);
        getClassStaticRef(gcRoots);
        getThreadRef(gcRoots);
        return gcRoots;
    }

    std::vector<OopHolder *> GarbageCollect::getHolders() const {
        std::vector<OopHolder *> oopHolders;
        oopHolders.emplace_back(&vm.oopManager->defaultOopHolder);
        for (const auto &item: vm.threadManager->getThreads()) {
            oopHolders.emplace_back(&item->oopHolder);
        }
        return oopHolders;
    }

    void GarbageCollect::processTrace(GarbageCollectContext &context) {
        //trace 有以下几个阶段
        //1. 先traceMark所有gcRoot
        //2. 有两类oop有可能幸存
        //  2.1 重写了finalize方法的 需要先执行finalize 所以不能回收 并且要将它traceMark 避免影响finalize的运行 
        //      但因为finalize只会执行一次 所以如果没有在finalize讲this和其他对象建立关系 则下次gc会回收
        //  2.2 VMThread对象 thread对象因为内部有OopHolder 所以暂时放在最后回收 如果要提前回收需要考虑将它的oopHolder
        //      里的对象先移动到别的地方

        const auto gcRoots = getGarbageCollectRoots();
        context.endGetRoots();

        for (const auto &item: gcRoots) {
            traceMarkOop(item);
        }

        const auto oopHolders = getHolders();
        for (const auto &holder : oopHolders) {
            for (const auto &oop : holder->oops) {
                if (enableFinalize) {
                    if (!oop->isFinalized() && oop->getType() == OopTypeEnum::INSTANCE_OOP) [[unlikely]] {
                        traceMarkOop(oop);
                        finalizeRunner.add(CAST_INSTANCE_OOP(oop));
                        continue;
                    }
                }

                if (oop->getClass()->getSpecialClassType() == SpecialClassEnum::THREAD_CLASS) {
                    traceMarkOop(oop);
                }
            }
        }

        context.endTraceOop();
    }

    void GarbageCollect::traceMarkOop(const ref oop) const {
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

    void GarbageCollect::traceMarkInstanceOopChild(const InstanceOop *oop) const {
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

    void GarbageCollect::traceMarkObjArrayOopChild(const ObjArrayOop *oop) const {
        if (const auto arrayLength = oop->getDataLength(); arrayLength > 0) {
            FOR_FROM_ZERO(arrayLength) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceMarkOop(element);
                }
            }
        }
    }

    void GarbageCollect::collectAll() {
        //这里被collect 则finalize方法无法被执行

        const auto oopHolders = getHolders();

        size_t deleteCount{0};
        std::vector<ref> lastCollect;
        for (const auto &item : oopHolders) {
            for (const auto &oop : item->oops) {
                const auto klass = oop->getClass();
                if (klass->getSpecialClassType() == SpecialClassEnum::THREAD_CLASS) {
                    //因为thread里有oopHolder 直接清理了会有问题
                    lastCollect.emplace_back(oop);
                    continue;
                }
                ++deleteCount;
                deleteOop(oop);
            }
        }

        for (const auto &item : lastCollect) {
            ++deleteCount;
            deleteOop(item);
        }
        (void)deleteCount;

#ifdef DEBUG
        if (enableGC) {
            cprintln("collectedMemory:{}KB, startCount:{}, successCount:{}", 
                CAST_F4(sumCollectedMemory) / 1024,
                collectStartCount,
                collectSuccessCount
            );

            cprintln("collectAll {}({}), oopHolder {}({}), oopDescSize:{}", 
                vm.oopManager->allocatedOopCount.load(), deleteCount,
                vm.oopManager->holders.size(), oopHolders.size(), vm.oopManager->ttDesc.size()
            );
        }
#endif
    }

    void GarbageCollect::join() {
        if (!enableGC) {
            return;
        }

        finalizeRunner.cv.notify_all();

        if (gcThread.joinable()) {
            gcThread.join();
        }
    }

    FinalizeRunner::FinalizeRunner(VM &vm, GarbageCollect &collector) : collector(collector)  {
    }

    void FinalizeRunner::add(InstanceOop *oop) {
        {
            std::lock_guard lock(dequeMtx);
            oopDeque.emplace_back(oop);
        }
        cv.notify_all();
    }

    void FinalizeRunner::runOopFinalize(InstanceOop *oop) const {
        const auto klass = oop->getInstanceClass();
        const auto finalizeMethod = klass->getMethod("finalize" "()V", false);
        if (finalizeMethod == nullptr) [[unlikely]] {
            cprintlnErr("run finalize error: get finalizeMethod fail");
        } else {
            createFrameAndRunMethod(*finalizeThread, *finalizeMethod, nullptr, {Slot(oop)});
        }
        oop->setFinalized(true);
    }

    bool FinalizeRunner::runOneOop() {
        InstanceOop *oop = nullptr;
        {
            std::unique_lock lock(dequeMtx);
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
                std::unique_lock lock(dequeMtx);
                cv.wait(lock, [this] { 
                    return !oopDeque.empty() 
                            || collector.checkStop
                            || collector.vm.exit; 
                });
            }
        }
    }

    void FinalizeRunner::initFinalizeThread(VMThread *mainThread) {
        if (!collector.enableGC) {
            return;
        }
        if (!collector.enableFinalize) {
            return;
        }

        const auto &vm = collector.vm;
        const auto threadClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        finalizeThread = CAST_VM_THREAD_OOP(vm.oopManager->newInstance(mainThread, threadClass));
        std::function func = [this] { runnerMethod(); };
        finalizeThread->addMethod(func);

        const auto threadConstructor = threadClass->getMethod("<init>" "(Ljava/lang/String;)V", false);
        const auto setDaemonMethod = threadClass->getMethod("setDaemon" "(Z)V", false);
        const auto threadNameOop = vm.stringPool->getInternString(mainThread, "Finalize Thread");

        const std::vector constructorParams = {
            Slot(finalizeThread), 
            Slot(threadNameOop) 
        };

        const std::vector setDaemonParams = {
            Slot(finalizeThread), 
            Slot(CAST_I4(1))
        };
        
        createFrameAndRunMethod(*mainThread, *threadConstructor, nullptr, constructorParams);
        createFrameAndRunMethod(*mainThread, *setDaemonMethod, nullptr, setDaemonParams);
        finalizeThread->start(nullptr, false);

    }


}