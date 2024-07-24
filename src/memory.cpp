#include "memory.hpp"
#include "vm.hpp"
#include "thread.hpp"
#include "class_loader.hpp"
#include "utils/format.hpp"

namespace RexVM {

    OopManager::OopManager(VM &vm) : vm(vm) {

    }

    InstanceOop *OopManager::newInstance(InstanceClass * const klass) {
        const auto oop = new InstanceOop(klass, klass->instanceSlotCount);
        allocatedOop.insert(oop);
        return oop;
    }

    VMThread *OopManager::newMainVMThread(Method &method, std::vector<Slot> params) {
        const auto threadClass = 
            vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);

        const auto threadGroupClass = 
            vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD_GROUP);
        const auto vmThreadGroup = newInstance(threadGroupClass);

        const auto oop = new VMThread(vm, threadClass, &method, params);
        oop->setFieldValue("group", "Ljava/lang/ThreadGroup;", Slot(vmThreadGroup));
        oop->setFieldValue("priority", "I", Slot(CAST_I8(1)));
        allocatedOop.insert(oop);
        return oop;
    }

    VMThread *OopManager::newVMThread(InstanceClass * const klass) {
        const auto oop = new VMThread(vm, klass, nullptr, {});
        allocatedOop.insert(oop);
        return oop;
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
                panic(cformat("error basic type: {}", CAST_U1(type)));
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

    ByteTypeArrayOop *OopManager::newByteArrayOop(size_t length, const u1 *initBuffer) {
        const auto oop = newByteArrayOop(length);
        std::copy(initBuffer, initBuffer + length, oop->data.get());
        return oop;
    }

    CharTypeArrayOop *OopManager::newCharArrayOop(size_t length) {
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(BasicType::T_CHAR);
        const auto oop = new CharTypeArrayOop(klass, length);
        allocatedOop.insert(oop);
        return oop;
    }

    InstanceOop *OopManager::newBooleanOop(i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_BOOLEAN);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "Z", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newByteOop(i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_BYTE);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "B", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newCharOop(i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_CHARACTER);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "C", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newShortOop(i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SHORT);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "S", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newIntegerOop(i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INTEGER);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "I", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newFloatOop(f4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_FLOAT);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "F", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newLongOop(i8 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_LONG);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "J", Slot(value));
        return integerOop;
    }

    InstanceOop *OopManager::newDoubleOop(f8 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_DOUBLE);
        const auto integerOop = newInstance(klass);
        integerOop->setFieldValue("value", "D", Slot(value));
        return integerOop;
    }


    void traceOop(Oop * const oop, std::unordered_set<Oop *> &tracedOop) {
        if (oop == nullptr) {
            return;
        }
        if (tracedOop.contains(oop)) {
            return;
        }
        tracedOop.insert(oop);

        const auto oopType = oop->type;
        switch (oopType) {
            case OopTypeEnum::InstanceOop:
                traceInstanceOopChild(CAST_INSTANCE_OOP(oop), tracedOop);
            break;

            case OopTypeEnum::ObjArrayOop:
                traceObjArrayOopChild(CAST_OBJ_ARRAY_OOP(oop), tracedOop);
            break;

            case OopTypeEnum::TypeArrayOop:
            return;
        }
        
    }

    void traceInstanceOopChild(InstanceOop * const oop, std::unordered_set<Oop *> &tracedOop) {
        const auto klass = oop->getInstanceClass();
        for (const auto &field : klass->fields) {
            const auto fieldType = field->getFieldSlotType();
            if (fieldType == SlotTypeEnum::REF && !field->isStatic()) {
                const auto memberOop = oop->getFieldValue(field->slotId).refVal;
                if (memberOop != nullptr) {
                    traceOop(memberOop, tracedOop);
                }
            }
        }
    }

    void traceObjArrayOopChild(ObjArrayOop * const oop, std::unordered_set<Oop *> &tracedOop) {
        const auto arrayLength = oop->dataLength;
        if (arrayLength > 0) {
            for (size_t i = 0; i < arrayLength; ++i) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceOop(element, tracedOop);
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

        cprintln("allcated {}", allocatedOop.size());

        const auto &classMap = classLoader->classMap;
        for (const auto &[key, klass] : classMap) {
            if (klass->type == ClassTypeEnum::InstanceClass) {
                auto const instanceClass = CAST_INSTANCE_CLASS(klass.get());
                for (const auto &field : instanceClass->fields) {
                    const auto fieldType = field->getFieldSlotType();
                    if (fieldType == SlotTypeEnum::REF && field->isStatic()) {
                        const auto memberOop = instanceClass->getFieldValue(field->slotId).refVal;
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

        cprintln("finish static and mirror {}", tracedOop.size());

        //Thread gc roots
        // const auto &threads = vm.threads;
        // for (const auto &thread : threads) {
        //     const auto threadGCRoots = thread->getThreadGCRoots();
        //     if (threadGCRoots.size() > 0) {
        //         tracedOop.insert(threadGCRoots.begin(), threadGCRoots.end());
        //     }
        // }

        cprintln("finish thread {}", tracedOop.size());

        std::unordered_set<Oop *> deleteOop;
//        std::copy_if(allocatedOop.begin(), allocatedOop.end(), std::inserter(deleteOop, deleteOop.end()),
//                 [&tracedOop](const auto& value) { return !tracedOop.contains(value) && !value->getNotGC(); });
//

        for (const auto &oop : deleteOop) {
            delete oop;
        }

        cprintln("{}, {}, {}", allocatedOop.size(), tracedOop.size(), deleteOop.size());

        oopManager->allocatedOop = tracedOop;
    }

    std::unordered_set<Oop *> getGCRoot(VM &vm) {
        std::unordered_set<Oop *> gcRoots;

        //From ClassLoader
        const auto &classLoader = vm.bootstrapClassLoader;
        if (classLoader != nullptr) {

        const auto &classMap = classLoader->classMap;
        for (const auto &[key, klass] : classMap) {
            if (klass->type == ClassTypeEnum::InstanceClass) {
                auto const instanceClass = CAST_INSTANCE_CLASS(klass.get());
                for (const auto &field : instanceClass->fields) {
                    const auto fieldType = field->getFieldSlotType();
                    if (fieldType == SlotTypeEnum::REF && field->isStatic()) {
                        const auto memberOop = instanceClass->getFieldValue(field->slotId).refVal;
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
        // const auto &threads = vm.threads;
        // for (const auto &thread : threads) {
        //     const auto &threadGCRoots = thread->getThreadGCRoots();
        //     if (threadGCRoots.size() > 0) {
        //         gcRoots.insert(threadGCRoots.begin(), threadGCRoots.end());
        //     }
        // }

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
        auto &allocatedOop = oopManager->allocatedOop;

        const auto &gcRoots = getGCRoot(vm);
        const auto &tracedOop = startTrace(gcRoots);

        cprintln("allocated {}, gcRoots {}, traced {}", allocatedOop.size(), gcRoots.size(), tracedOop.size());


        std::unordered_set<Oop *> notTracedOop;
//        std::copy_if(allocatedOop.begin(), allocatedOop.end(), std::inserter(notTracedOop, notTracedOop.end()),
//                 [&tracedOop](const auto& value) { return !tracedOop.contains(value) && !value->getNotGC(); });
//
        cprintln("notTracedOop {}", notTracedOop.size());
        for (const auto &oop : notTracedOop) {
            delete oop;
            allocatedOop.erase(oop);
        }

        // for (const auto &oop : allocatedOop) {
        //     if (oop != nullptr) {
        //     delete oop;
        //     }
        // }

        // for (const auto &oop : tracedOop) {
        //     const auto klass = oop->klass;
        //     // const auto mirrorOop = CAST_MIRROR_OOP(oop);
        //     // const auto name = klass->name;
        //     // const auto mklass = mirrorOop->mirrorClass;

        //     //cprintln("{}", mklass->name);
        //     // if (name != "java/lang/Class") {
        //     //     cprintln("{}", klass->name);
        //     //     cnt++;
        //     // }
        // }


    }
    
}