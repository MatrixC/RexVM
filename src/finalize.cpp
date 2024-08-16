#include "finalize.hpp"
#include <chrono>
#include "vm.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "string_pool.hpp"
#include "print_helper.hpp"
#include "frame.hpp"

namespace RexVM {
    //是否可以加调规则 如果当前调用栈中有native函数 就不进行gc
    //gc线程里尽量不要再分配heap内存

    //先不回收Mirror
    extern void *jjjj;
    Collector::Collector(VM &vm) : vm(vm) {
        collectThread = std::thread([this]() {
            while (!this->vm.exit) {
                if (oopCount > 1000) {
                    //startGC();
                }
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
        });
    }

    void Collector::stopTheWorld() {
        std::lock_guard<std::mutex> lock(mtx);
        markCollect = true;


        while (!vm.checkAllThreadStopForCollect()) {
        }
    }

    void Collector::collectFinish() {
        stopThreadCount = 0;
        markCollect = false;
        cv.notify_all();
    }

    void Collector::checkStop(Frame &frame, const cstring &methodName) {
        if (!markCollect) [[likely]] {
            return;
        }

        ++stopThreadCount;
        std::unique_lock<std::mutex> lock(mtx);
        cprintln("stop methodName: {}", methodName);
        //frame.printCallStack();
        frame.thread.stopForCollect = true;
        cv.wait(lock, [this] { return !markCollect; });
        frame.thread.stopForCollect = false;
    }

    std::unordered_map<ref, cstring> deletedMap;

    void deleteItem(ref item) {
        if (item->getClass()->name == JAVA_LANG_STRING_NAME) {
            const auto k1 = CAST_INSTANCE_OOP(item);
            auto jstr = StringPool::getJavaString(k1);
            bool kk = jstr == "";
            const auto arr = CAST_CHAR_TYPE_ARRAY_OOP(k1->getFieldValue("value", "[C").refVal);
            const auto k2 = cformat("{}:{}:{}:{}", "STR", jstr, std::to_string(jstr.size()), CAST_VOID_PTR(arr));
            deletedMap.emplace(item, k2); 
        } else {
            deletedMap.emplace(item, item->getClass()->name);
        }

        delete item;
        
    }

    void showItem(ref item) {
        const auto iter = deletedMap.find(item);
        if (iter != deletedMap.end()) {
            cprintln("{}", iter->second);
        } else {
            cprintln("null");
        }
    }

    void Collector::startGC() {
        cprintln("start gc");
        stopTheWorld();
        //cprintln("stop the world finish");
        // const auto &gcRoots = getGcRoots(vm);
        // for (const auto &item : gcRoots) {
        //     traceOop(item);
        // }

        for (const auto &thread : vm.vmThreadDeque) {
            if (thread->hasNativeCall()) {
                collectFinish();
                return;
            }
        }


        std::vector<ref> gcRoots;
        getStaticRef(*vm.bootstrapClassLoader, gcRoots);
        for (const auto &item : gcRoots) {
            traceOop(item);
        }
        gcRoots.clear();
        getThreadRef(vm, gcRoots);
        for (const auto &item : gcRoots) {
            traceOop(item);
        }

        //cprintln("trace gcroot finish");
        const auto stringClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING);

        std::vector<ref> liveRefs;
        liveRefs.reserve(100000);

        for (const auto &item : vm.oopManager->defaultOopHolder.oops) {
            if (item->isTraced()) {
                liveRefs.emplace_back(item);
                item->clearTraced();
            } else {
                if (item->getClass() == stringClass) {
                    vm.stringPool->gcStringOop(CAST_INSTANCE_OOP(item));
                }
                deleteItem(item);
            }
        }
        vm.oopManager->defaultOopHolder.oops.clear();

        for (const auto &thread : vm.vmThreadDeque) {
            for (const auto &item : thread->oopHolder.oops) {
                if (item->isTraced()) {
                    liveRefs.emplace_back(item);
                    item->clearTraced();
                } else {
                    if (item->getClass() == stringClass) {
                        vm.stringPool->gcStringOop(CAST_INSTANCE_OOP(item));
                    }
                    if (item->getClass()->name == JAVA_LANG_REFLECT_FIELD_NAME) {
                        int i = 10;
                    }
                    deleteItem(item);
                }
            }
            thread->oopHolder.oops.clear();
        }

        vm.oopManager->defaultOopHolder.oops = liveRefs;
        


        // u8 deleteCnt{0};
        // collect(vm, vm.oopManager->defaultOopHolder, liveRefs, deleteCnt);
        // for (const auto &thread : vm.vmThreadDeque) {
        //     collect(vm, thread->oopHolder, liveRefs, deleteCnt);
        // }
        // //cprintln("collect finish {}", deleteCnt);

        // for (const auto &item : liveRefs) {
        //     // auto isLong = false;
        //     if (item->getClass()->name == JAVA_LANG_CLASS_NAME) {
        //         const auto mirrorOop = CAST_MIRROR_OOP(item);
        //         const auto klass = mirrorOop->getMirrorClass();
        //         cprintln("LIVE, {}", klass->name);
        //     }
        //     item->clearTraced();
            
        //     // if (isLong) {
        //     //    cprintln("AAAAA, {}", item->isMarkTraced()); 
        //     // }
        // }

        // vm.oopManager->defaultOopHolder.oops = liveRefs;

        oopCount = 0;
        cprintln("gc finish {}", liveRefs.size());

        collectFinish();
    }

    void Collector::join() {
        if (collectThread.joinable()) {
            collectThread.join();
        }
    }
    
    void gcClass() {
        //保留class规则
        //1. 这个class有对应的oop
        //2. 这个class的mirror有被引用
    }

    void traceInstanceOopChild(InstanceOop *oop) {
        //如果是mirrorOop 要判断是否是ConstantPool Mirror 它里面包含了一个Class的MirrorOop

        const auto klass = oop->getInstanceClass();
        //要包含它父类的字段
        for (auto current = klass; current != nullptr; current = current->superClass) {
            for (const auto &field : current->fields) {
                const auto fieldType = field->getFieldSlotType();
                if (fieldType == SlotTypeEnum::REF && !field->isStatic()) {
                    const auto memberOop = oop->getFieldValue(field->slotId).refVal;
                    if (memberOop != nullptr) {
                        if (field->name == "name" && klass->name == JAVA_LANG_CLASS_NAME) {
                            int i = 10;
                            //cprintln("trace name filed {} {}", klass->name, StringPool::getJavaString(CAST_INSTANCE_OOP(memberOop)));
                        }
                        traceOop(memberOop);
                    }
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
        if (jjjj == CAST_VOID_PTR(oop)) {
            int i = 10;
        }

        if (oop->getClass()->name == JAVA_LANG_CLASS_NAME) {
            const auto mirrorOop = CAST_MIRROR_OOP(oop);
            const auto klass = mirrorOop->getMirrorClass();
            if (klass->name == "java/lang/Long") {
                //cprintln("traceOop Long, {} {}", oop->isMarkTraced(), CAST_VOID_PTR(oop));
            }
            // const auto name = CAST_VOID_PTR(mirrorOop->getFieldValue("name", "Ljava/lang/String;").refVal);
            // cprintln("trace to class {}, name {}", klass->name, name);

        }

        if (oop == nullptr || oop->isTraced()) {
            return;
        }

        if (oop->getClass()->name == JAVA_LANG_CLASS_NAME) {
            int i = 10;
            const auto mirrorOop = CAST_MIRROR_OOP(oop);
            const auto klass = mirrorOop->getMirrorClass();
            const auto name = CAST_VOID_PTR(mirrorOop->getFieldValue("name", "Ljava/lang/String;").refVal);
            //cprintln("trace to class {}, name {}", klass->name, name);

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
            const auto mirror = klass->getMirror(nullptr, false);
            if (mirror != nullptr) {
                gcRoots.emplace_back(mirror);
            }
            if (klass->type == ClassTypeEnum::INSTANCE_CLASS) {
                const auto instanceClass = CAST_INSTANCE_CLASS(klass.get());
                if (instanceClass->notInitialize()) {
                    continue;
                }

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
                gcRoots.emplace_back(thread);
            }
        }
    }

    std::vector<ref> getGcRoots(VM &vm) {
        std::vector<ref> gcRoots;
        gcRoots.reserve(20000);
        getStaticRef(*vm.bootstrapClassLoader, gcRoots);
        getThreadRef(vm, gcRoots);
        return gcRoots;
    }

    void collect(VM &vm, std::vector<ref> &currentRefs, std::vector<ref> &liveRefs, u8 &deleteCnt) {
        const auto stringClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING);
        for (const auto &item : currentRefs) {
            if (item->isTraced() || item->getClass()->name == "[C") {
                liveRefs.emplace_back(item);
                item->clearTraced();
            } else {
                if (item->getClass() == stringClass) {
                    cprintln("gcs {}", StringPool::getJavaString(CAST_INSTANCE_OOP(item)));
                    vm.stringPool->gcStringOop(CAST_INSTANCE_OOP(item));
                }

                delete item;
                ++deleteCnt;
            }
        }
    }

    void collect(VM &vm, OopHolder &oopHolder, std::vector<ref> &liveRefs, u8 &deleteCnt) {
        collect(vm, oopHolder.oops, liveRefs, deleteCnt); 
        oopHolder.clear(); 
    }



}