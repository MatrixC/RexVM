#include "memory.hpp"
#include "vm.hpp"
#include "runtime.hpp"
#include "class_loader.hpp"
#include "utils/format.hpp"
#include "vm.hpp"
#include <algorithm>

namespace RexVM {

    OopManager::OopManager(VM &vm) : vm(vm) {

    }

    InstanceOop *OopManager::newInstance(InstanceClass * const klass) {
        const auto oop = new InstanceOop(klass, klass->instanceSlotCount);
        allocatedOop.insert(oop);
        return oop;
    }

    ThreadOop *OopManager::newThreadOop(Thread *thread) {
        const auto threadClass = vm.bootstrapClassLoader->getInstanceClass("java/lang/Thread");
        const auto threadGroupClass = vm.bootstrapClassLoader->getInstanceClass("java/lang/ThreadGroup");

        const auto vmThreadOop = new ThreadOop(threadClass, thread);
        const auto vmThreadGroup = newInstance(threadGroupClass);

        vmThreadOop->setFieldValue("group", "Ljava/lang/ThreadGroup;", Slot(vmThreadGroup));
        vmThreadOop->setFieldValue("priority", "I", Slot(1));

        allocatedOop.insert(vmThreadOop);
        return vmThreadOop;
    }

    ObjArrayOop *OopManager::newObjArrayOop(ObjArrayClass * const klass, size_t length) {
        const auto oop = new ObjArrayOop(klass, length);
        allocatedOop.insert(oop);
        return oop;
    }

    TypeArrayOop *OopManager::newTypeArrayOop(BasicType type, size_t length) {
        TypeArrayOop *oop = nullptr;
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(type);
        switch (type) {
            case BasicType::T_BOOLEAN:
            case BasicType::T_BYTE:
                oop = new ByteTypeArrayOop(klass, length);
                break;

            case BasicType::T_SHORT:
                oop = new ShortTypeArrayOop(klass, length);
                break;

            case BasicType::T_INT:
                oop = new IntTypeArrayOop(klass, length);
                break;

            case BasicType::T_LONG:
                oop = new LongTypeArrayOop(klass, length);
                break;

            case BasicType::T_CHAR:
                oop = new CharTypeArrayOop(klass, length);
                break;

            case BasicType::T_FLOAT:
                oop = new FloatTypeArrayOop(klass, length);
                break;

            case BasicType::T_DOUBLE:
                oop = new DoubleTypeArrayOop(klass, length);
                break;

            default:
                panic(format("error basic type: {}", static_cast<u1>(type)));
                break;
        }

        allocatedOop.insert(oop);
        return oop;
    }

    ByteTypeArrayOop *OopManager::newByteArrayOop(size_t length) {
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(BasicType::T_BYTE);
        const auto oop = new ByteTypeArrayOop(klass, length);
        allocatedOop.insert(oop);
        return oop;
    }

    ByteTypeArrayOop *OopManager::newByteArrayOop(size_t length, u1 *initBuffer) {
        const auto oop = newByteArrayOop(length);
        for (auto i = 0; i < length; ++i) {
            oop->data[i] = initBuffer[i];
        }
        return oop;
    }

    CharTypeArrayOop *OopManager::newCharArrayOop(size_t length) {
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(BasicType::T_CHAR);
        const auto oop = new CharTypeArrayOop(klass, length);
        allocatedOop.insert(oop);
        return oop;
    }


    void traceOop(Oop * const oop, std::unordered_set<Oop *> &tracedOop) {
        if (oop == nullptr) {
            return;
        }
        if (tracedOop.contains(oop)) {
            return;
        }
        tracedOop.insert(oop);

        const auto oopTpye = oop->type;
        switch (oopTpye) {
            case OopTypeEnum::InstanceOop:
                traceInstanceOop(static_cast<InstanceOop *>(oop), tracedOop);
            break;

            case OopTypeEnum::ObjArrayOop:
                traceObjArrayOop(static_cast<ObjArrayOop *>(oop), tracedOop);
            break;

            case OopTypeEnum::TypeArrayOop:
            return;
        }
        
    }

    void traceInstanceOop(InstanceOop * const oop, std::unordered_set<Oop *> &tracedOop) {
        const auto klass = static_cast<InstanceClass *>(oop->klass);
        for (const auto &field : klass->fields) {
            const auto fieldType = field->getFieldSlotType();
            if (fieldType == SlotTypeEnum::REF && !field->isStatic()) {
                const auto memberOop = static_cast<Oop *>(oop->getFieldValue(field->slotId).refVal);
                if (memberOop != nullptr) {
                    traceOop(memberOop, tracedOop);
                }
            }
        }
    }

    void traceObjArrayOop(ObjArrayOop * const oop, std::unordered_set<Oop *> &tracedOop) {
        const auto arrayLength = oop->dataLength;
        if (arrayLength > 0) {
            for (size_t i = 0; i < arrayLength; ++i) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceOop(static_cast<Oop *>(element), tracedOop);
                }
            }
        }
    }

    void gc(VM &vm) {
        std::unordered_set<Oop *> tracedOop;
        const auto &oopManager = vm.oopManager;
        const auto &allocatedOop = oopManager->allocatedOop;


        //ClassLoader
        const auto &classLoader = vm.bootstrapClassLoader;
        // if (classLoader->classLoaderInstance != nullptr) {
        //     tracedOop.insert(classLoader->classLoaderInstance.get());
        // }
        
        //InstanceClass static field and Class Mirror

        println("allcated {}", allocatedOop.size());

        const auto &classMap = classLoader->classMap;
        for (const auto &[key, klass] : classMap) {
            if (klass->type == ClassTypeEnum::InstanceClass) {
                auto const instanceClass = static_cast<InstanceClass *>(klass.get());
                for (const auto &field : instanceClass->fields) {
                    const auto fieldType = field->getFieldSlotType();
                    if (fieldType == SlotTypeEnum::REF && field->isStatic()) {
                        const auto memberOop = static_cast<Oop *>(instanceClass->getFieldValue(field->slotId).refVal);
                        if (memberOop != nullptr) {
                            tracedOop.insert(memberOop);
                        }
                    }
                }
            }

            const auto &classMirror = klass->mirror;
            if (classMirror != nullptr) {
                tracedOop.insert(classMirror.get());
            }
        }

        println("finish static and mirror {}", tracedOop.size());

        //Thread gc roots
        const auto &threads = vm.executor->threads;
        for (const auto &thread : threads) {
            const auto threadGCRoots = thread->getThreadGCRoots();
            if (threadGCRoots.size() > 0) {
                tracedOop.insert(threadGCRoots.begin(), threadGCRoots.end());
            }
        }

        println("finish thread {}", tracedOop.size());

        std::unordered_set<Oop *> deleteOop;
        std::copy_if(allocatedOop.begin(), allocatedOop.end(), std::inserter(deleteOop, deleteOop.end()),
                 [&tracedOop](const auto& value) { return !tracedOop.contains(value) && !value->getNotGC(); });


        for (const auto &oop : deleteOop) {
            delete oop;
        }

        println("{}, {}, {}", allocatedOop.size(), tracedOop.size(), deleteOop.size());

        oopManager->allocatedOop = tracedOop;
    }

    std::unordered_set<Oop *> getGcRoot(VM &vm) {
        std::unordered_set<Oop *> gcRoots;

        //From ClassLoader
        const auto &classLoader = vm.bootstrapClassLoader;
        if (classLoader != nullptr) {

        const auto &classMap = classLoader->classMap;
        for (const auto &[key, klass] : classMap) {
            if (klass->type == ClassTypeEnum::InstanceClass) {
                auto const instanceClass = static_cast<InstanceClass *>(klass.get());
                for (const auto &field : instanceClass->fields) {
                    const auto fieldType = field->getFieldSlotType();
                    if (fieldType == SlotTypeEnum::REF && field->isStatic()) {
                        const auto memberOop = static_cast<Oop *>(instanceClass->getFieldValue(field->slotId).refVal);
                        if (memberOop != nullptr) {
                            gcRoots.insert(memberOop);
                        }
                    }
                }
            }

            const auto &classMirror = klass->mirror;
            if (classMirror != nullptr) {
                gcRoots.insert(classMirror.get());
            }
        }

        }

        //From Thread
        const auto &threads = vm.executor->threads;
        for (const auto &thread : threads) {
            const auto &threadGCRoots = thread->getThreadGCRoots();
            if (threadGCRoots.size() > 0) {
                gcRoots.insert(threadGCRoots.begin(), threadGCRoots.end());
            }
        }

        return gcRoots;

    }

    std::unordered_set<Oop *> startTrace(std::unordered_set<Oop *> const &gcRoots) {
        std::unordered_set<Oop *> tracedOop;
        for (const auto &oop : gcRoots) {
            traceOop(oop, tracedOop);
        }
        return tracedOop;
    }

    void gc2(VM &vm) {
        const auto &oopManager = vm.oopManager;
        const auto &allocatedOop = oopManager->allocatedOop;

        const auto &gcRoots = getGcRoot(vm);
        const auto &tracedOop = startTrace(gcRoots);

        println("allocated {}, gcRoots {}, traced {}", allocatedOop.size(), gcRoots.size(), tracedOop.size());


        std::unordered_set<Oop *> notTracedOop;
        std::copy_if(allocatedOop.begin(), allocatedOop.end(), std::inserter(notTracedOop, notTracedOop.end()),
                 [&tracedOop](const auto& value) { return !tracedOop.contains(value) && !value->getNotGC(); });

        println("notTracedOop {}", notTracedOop.size());
        for (const auto &oop : notTracedOop) {
            delete oop;
        }

        // for (const auto &oop : allocatedOop) {
        //     if (oop != nullptr) {
        //     delete oop;
        //     }
        // }

        // for (const auto &oop : tracedOop) {
        //     const auto klass = oop->klass;
        //     // const auto mirrorOop = static_cast<MirrorOop *>(oop);
        //     // const auto name = klass->name;
        //     // const auto mklass = mirrorOop->mirrorClass;

        //     //println("{}", mklass->name);
        //     // if (name != "java/lang/Class") {
        //     //     println("{}", klass->name);
        //     //     cnt++;
        //     // }
        // }


    }
    
}