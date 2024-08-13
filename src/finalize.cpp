#include "finalize.hpp"
#include "vm.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "memory.hpp"

namespace RexVM {

    Collector::Collector(VM &vm) : vm(vm) {
    }

    void Collector::stopTheWorld() {
        std::lock_guard<std::mutex> lock(mtx);
        markCollect = true;

        while(vm.getActiveThreadCount() != stopThreadCount) {
        }
    }

    void Collector::collectFinish() {
        stopThreadCount = 0;
        markCollect = false;
        cv.notify_all();
    }

    void Collector::checkStop() {
        if (!markCollect) [[likely]] {
            return;
        }

        ++stopThreadCount;
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !markCollect; });
    }

    void Collector::startGC() {
        cprintln("start gc");
        stopTheWorld();
        cprintln("stop the world finish");
        const auto &gcRoots = getGcRoots(vm);
        for (const auto &item : gcRoots) {
            traceOop(item);
        }
        std::vector<ref> liveRefs;
        liveRefs.reserve(100000);
        collect(vm.oopManager->defaultOopHolder, liveRefs);
        for (const auto &thread : vm.vmThreadDeque) {
            collect(thread->oopHolder, liveRefs);
        }

        for (const auto &item : liveRefs) {
            item->clearTraced();
        }

        vm.oopManager->defaultOopHolder.oops = liveRefs;

        oopCount = 0;
        cprintln("gc finish {}", liveRefs.size());

        collectFinish();
    }

    void traceInstanceOopChild(InstanceOop *oop) {
        const auto klass = oop->getInstanceClass();
        for (const auto &field : klass->fields) {
            const auto fieldType = field->getFieldSlotType();
            if (fieldType == SlotTypeEnum::REF && !field->isStatic()) {
                const auto memberOop = oop->getFieldValue(field->slotId).refVal;
                if (memberOop != nullptr) {
                    traceOop(memberOop);
                }
            }
        }
    }

    void traceObjArrayOopChild(ObjArrayOop * oop) {
        const auto arrayLength = oop->getDataLength();
        if (arrayLength > 0) {
            FOR_FROM_ZERO(arrayLength) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceOop(element);
                }
            }
        }
    }

    void traceOop(ref oop) {
        if (oop == nullptr || oop->isMarkTraced()) {
            return;
        }

        oop->markTraced();
        switch (oop->getType()) {
            case OopTypeEnum::INSTANCE_OOP:
                traceInstanceOopChild(CAST_INSTANCE_OOP(oop));
                break;

            case OopTypeEnum::OBJ_ARRAY_OOP:
                traceObjArrayOopChild(CAST_OBJ_ARRAY_OOP(oop));
                break;

            default:
                return;
        }
    }

    // void collectOop(ref oop) {
    //     const auto klass = oop->getClass();
    //     if (klass->getType() == ClassTypeEnum::INSTANCE_CLASS) {
    //         const auto instanceClass = CAST_INSTANCE_CLASS(klass);
    //         const auto finalizeMethod = instanceClass->getMethodSelf("finalize", "()V", false);
    //         if (finalizeMethod != nullptr) {
                
    //         }
    //     }
    // }

    // void collectAll(VM &vm) {
    //     const auto &oopManager = vm.oopManager;
    //     auto &allocatedOop = oopManager->allocatedOop;

    //     for (auto &oop : allocatedOop) {
    //         delete oop;
    //     }
    // }

    void getStaticRef(ClassLoader &classLoader, std::vector<ref> &gcRoots) {
        for (const auto &[name, klass] : classLoader.classMap) {
            if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
                const auto instanceClass = CAST_INSTANCE_CLASS(klass.get());
                for (const auto &field : instanceClass->fields) {
                    if (field->getFieldSlotType() == SlotTypeEnum::REF && field->isStatic()) {
                        if (const auto oop = instanceClass->getFieldValue(field->slotId).refVal; oop != nullptr) {
                            gcRoots.emplace_back(oop);
                        }
                    }
                }
            }
        }
    }

    void getThreadRef(VM &vm, std::vector<ref> &gcRoots) {
        for (const auto &thread : vm.vmThreadDeque) {
            const auto status = thread->getStatus();
            if (status != ThreadStatusEnum::TERMINATED) {
                thread->getThreadGCRoots(gcRoots);
            }
            gcRoots.emplace_back(thread);
        }
    }

    std::vector<ref> getGcRoots(VM &vm) {
        std::vector<ref> gcRoots;
        gcRoots.reserve(20000);
        getStaticRef(*vm.bootstrapClassLoader, gcRoots);
        getThreadRef(vm, gcRoots);
        return gcRoots;
    }

    void collect(std::vector<ref> &currentRefs, std::vector<ref> &liveRefs) {
        for (const auto &item : currentRefs) {
            if (item->isMarkTraced()) {
                liveRefs.emplace_back(item);
            } else {
                delete item;
            }
        }
    }

    void collect(OopHolder &oopHolder, std::vector<ref> &liveRefs) {
        collect(oopHolder.oops, liveRefs); 
        oopHolder.clear(); 
    }



}