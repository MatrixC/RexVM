#include "garbage_collect.hpp"
#include <unordered_set>
#include <algorithm>
#include "vm.hpp"
#include "thread.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "class_loader.hpp"
#include "string_pool.hpp"
#include "memory.hpp"
#include "utils/string_utils.hpp"
#include "execute.hpp"
#include "os_platform.hpp"

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
        //如果有线程栈中有native函数 停止gc
        for (const auto &thread: vm.vmThreadDeque) {
            if (thread->hasNativeCall()) {
                return false;
            }
        }
        //可以继续gc
        return true;
    }

    void GarbageCollect::stopTheWorld() {
        markCollect = true;
        finalizeRunner.cv.notify_all();
        while (!vm.checkAllThreadStopForCollect()) {
        }
    }

    void GarbageCollect::startTheWorld() {
        markCollect = false;
        cv.notify_all();
    }

    void GarbageCollect::run() {
        stopTheWorld();
        if (!checkTerminationCollect()) {
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
        for (const auto &item: vm.vmThreadDeque) {
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
                if (oop->isMirror()) [[unlikely]] {
                    //run ~MirOop
                    delete CAST_MIRROR_OOP(oop);
                } else {
                    delete oop;
                }
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
        for (const auto &thread: vm.vmThreadDeque) {
            const auto status = thread->getStatus();
            if (status != ThreadStatusEnum::TERMINATED) {
                thread->getThreadGCRoots(gcRoots);
                gcRoots.emplace_back(thread);
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

    void GarbageCollect::collectAll() const {
        cprintln("sumCollectedMemory:{}KB", CAST_F4(sumCollectedMemory) / 1024);
        for (const auto &oop: vm.oopManager->defaultOopHolder.oops) {
            delete oop;
        }

        for (const auto &item: vm.vmThreadDeque) {
            for (const auto &oop: item->oopHolder.oops) {
                delete oop;
            }
        }
    }

    void GarbageCollect::start() {
        stringClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING);

        gcThread = std::thread([this]() {
            setThreadName("GC Thread");
            while (!this->vm.exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(collectSleepTime));
                if (vm.oopManager->allocatedOopMemory > collectMemoryThreshold) {
                    this->run();
                }
            }
        });

        finalizeRunner.start();
    }

    void GarbageCollect::join() {
        finalizeRunner.thread->join();

        if (gcThread.joinable()) {
            gcThread.join();
        }
    }

    FinalizeRunner::FinalizeRunner(VM &vm, GarbageCollect &collector) : collector(collector)  {
        thread = std::unique_ptr<VMThread>(VMThread::createOriginVMThread(vm));
        thread->setName("Finalize Thread");
    }

    void FinalizeRunner::add(InstanceOop *oop) {
        {
            std::lock_guard<std::mutex> lock(dequeMtx);
            oopDeque.emplace_back(oop);
        }
        cv.notify_all();
    }

    void FinalizeRunner::run(InstanceOop *oop) const {
        const auto klass = oop->getInstanceClass();
        const auto finalizeMethod = klass->getMethod("finalize", "()V", false);
        if (finalizeMethod == nullptr) {
            panic("run finalize error");
        }
        createFrameAndRunMethod(*thread, *finalizeMethod, {Slot(oop)}, nullptr);
        oop->setFinalized(true);
    }

    void FinalizeRunner::start() {
        std::function<void()> runnable = [this]() {
            while (!collector.vm.exit) {
                {
                    std::unique_lock<std::mutex> lock(dequeMtx);
                    if (oopDeque.empty()) {
                        cv.wait(lock, [this] { return !oopDeque.empty() || collector.markCollect; });
                    }
                }

                if (oopDeque.empty()) {
                    collector.checkStopForCollect(*thread);
                } else {
                    std::unique_lock<std::mutex> lock(dequeMtx);
                    while (!oopDeque.empty()) {
                        collector.checkStopForCollect(*thread);
                        const auto oop = oopDeque.front();
                        oopDeque.pop_front();
                        run(oop);
                    }
                }
            }
        };

        thread->addMethod(runnable);
        //Don't insert into VM->threadDeque, gc thread join this
        thread->start(nullptr, false);
    }

}