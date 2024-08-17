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

    GarbageCollect::GarbageCollect(VM &vm) : vm(vm) {
    }

    void GarbageCollect::checkStopForCollect(Frame &frame) {
        if (!markCollect) [[likely]] {
            return;
        }

        std::unique_lock<std::mutex> lock(mtx);
        frame.thread.stopForCollect = true;
        cv.wait(lock, [this] { return !markCollect; });

//        auto holder = frame.thread.oopHolder;
//        if (!holder.runFinalizeOops.empty()) {
//            FOR_VECTOR_ITEM(holder.runFinalizeOops) {
//                runFinalize(frame, CAST_INSTANCE_OOP(item));
//            }
//            holder.clearFinalizeOop();
//        }
        frame.thread.stopForCollect = false;
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
        if (enableLog) {
            context.printLog(vm);
        }


//        std::vector<ref> survives;
//        survives.reserve(GC_SURVIVE_SIZE);
//
//        auto &defaultOopHolder = vm.oopManager->defaultOopHolder;
//
//        size_t allocatedOopCount = defaultOopHolder.oops.size();
//        size_t allocatedOopMemory{0};
//        size_t collectedOopMemory{0};
//        for (const auto &oop: defaultOopHolder.oops) {
//            allocatedOopMemory += oop->getMemorySize();
//            collect(oop, survives, collectedOopMemory);
//        }
//        defaultOopHolder.clear();
//
//        for (const auto &item: vm.vmThreadDeque) {
//            auto &holder = item->oopHolder;
//            allocatedOopCount += holder.oops.size();
//            for (const auto &oop: holder.oops) {
//                allocatedOopMemory += oop->getMemorySize();
//                collect(oop, survives, collectedOopMemory);
//            }
//            holder.clear();
//        }
//        const auto remainOopCount = survives.size();
//        const auto collectedOopCount = allocatedOopCount - remainOopCount;
//        const auto remainOopMemory = allocatedOopMemory - collectedOopMemory;
//        sumCollectedMemory += collectedOopMemory;

        //fuck!
        //or gcRoots all clearTraced()
        vm.mainThread->clearTraced();

//        vm.oopManager->defaultOopHolder.oops = survives;

//        if (enableLog) {
//            cprintln("gcLog gcRootCount:{}, oopCount:{}, oopMemory:{}KB, collectCount:{}, collectMemory:{}KB",
//                     gcRoots.size(), allocatedOopCount, CAST_F4(allocatedOopMemory) / 1024, collectedOopCount, CAST_F4(collectedOopMemory) / 1024);
//        }

        startTheWorld();
    }

    void GarbageCollect::collect(ref oop, std::vector<ref> &survives, size_t &collectedOopMemory) const {
        if (oop->isTraced()) {
            survives.emplace_back(oop);
            oop->clearTraced();
        } else {
            if (!oop->isFinalized()) [[unlikely]] {
                //need to run finalize
                survives.emplace_back(oop);
                //run finalize

                oop->setFinalized(true);
                return;
            }

            const auto oopClass = oop->getClass();
            collectedOopMemory += oop->getMemorySize();
            if (oopClass == stringClass) {
                vm.stringPool->gcStringOop(CAST_INSTANCE_OOP(oop));
            }
            if (oop->isMirror()) {
                //run ~MirOop
                delete CAST_MIRROR_OOP(oop);
            } else {
                delete oop;
            }
        }
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
//                if (!oop->isFinalized()) [[unlikely]] {
//                    //need to run finalize
//                    survives.emplace_back(oop);
//                    //run finalize
//                    holder.addFinalizeOop(oop);
//                    continue;
//                }

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

    void GarbageCollect::runFinalize(Frame &frame, InstanceOop *oop) const {
        const auto klass = oop->getInstanceClass();
        const auto finalizeMethod = klass->getMethod("finalize", "()V", false);
        if (finalizeMethod == nullptr) {
            panic("run finalize error");
        }

        frame.runMethodManual(*finalizeMethod, {Slot(oop)});
        if (frame.markThrow) {
            return;
        }
        oop->setFinalized(true);
    }

    void GarbageCollect::start() {
        constantPoolClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::SUN_REFLECT_CONSTANT_POOL);
        stringClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING);

        gcThread = std::thread([this]() {
            while (!this->vm.exit) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                this->run();
            }
        });
    }

    void GarbageCollect::join() {
        if (gcThread.joinable()) {
            gcThread.join();
        }
    }

}