#include "memory.hpp"

#include "vm.hpp"
#include "thread.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "class_loader.hpp"
#include "string_pool.hpp"
#include "utils/format.hpp"

namespace RexVM {

    std::atomic_int oopCount{0};

    OopHolder::OopHolder(size_t size) {
        oops.reserve(size);
    }

    OopHolder::OopHolder() : OopHolder(8192) {
    }

    void OopHolder::addOop(ref oop) {
        oops.emplace_back(oop);
    }

    void OopHolder::clear() {
        oops.clear();
        oops.shrink_to_fit();
    }

    OopManager::OopManager(VM &vm) : vm(vm) {
    }

    InstanceOop *OopManager::newInstance(VMThread *thread, InstanceClass * klass) {
        const auto oop = new InstanceOop(klass, klass->instanceSlotCount);
        addToOopHolder(thread, oop);
        return oop;
    }

    MirOop *OopManager::newMirror(VMThread *thread, InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type) {
        if (klass == nullptr) {
            switch (type) {
                case MirrorObjectTypeEnum::CLASS:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_CLASS);
                    break;

                case MirrorObjectTypeEnum::METHOD:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_METHOD);
                    break;

                case MirrorObjectTypeEnum::CONSTRUCTOR:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_CONSTRUCTOR);
                    break;

                case MirrorObjectTypeEnum::FIELD:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_FIELD);
                    break;

                case MirrorObjectTypeEnum::MEMBER_NAME:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INVOKE_MEMBER_NAME);
                    break;

                case MirrorObjectTypeEnum::CONSTANT_POOL:
                    klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::SUN_REFLECT_CONSTANT_POOL);
                    break;

                default:
                    panic("error type");
            }
        }
        const auto oop = new MirOop(klass, mirror, type);
        addToOopHolder(thread, oop);
        return oop;
    }

    VMThread *OopManager::newVMThread(VMThread *thread, InstanceClass * const klass) {
        const auto oop = new VMThread(vm, klass);
        addToOopHolder(thread, oop);
        return oop;
    }

    ObjArrayOop *OopManager::newObjArrayOop(VMThread *thread, ObjArrayClass * const klass, size_t length) {
        const auto oop = new ObjArrayOop(klass, length);
        addToOopHolder(thread, oop);
        return oop;
    }

    ObjArrayOop *OopManager::newObjectObjArrayOop(VMThread *thread, size_t length) {
        const auto klass = vm.bootstrapClassLoader->getObjectArrayClass(JAVA_LANG_OBJECT_NAME);
        return newObjArrayOop(thread, klass, length);
    }

    ObjArrayOop *OopManager::newClassObjArrayOop(VMThread *thread, size_t length) {
        const auto klass = vm.bootstrapClassLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        return newObjArrayOop(thread, klass, length);
    }

    ObjArrayOop *OopManager::newStringObjArrayOop(VMThread *thread, size_t length) {
        const auto klass = vm.bootstrapClassLoader->getObjectArrayClass(JAVA_LANG_STRING_NAME);
        return newObjArrayOop(thread, klass, length);
    }

    TypeArrayOop *OopManager::newTypeArrayOop(VMThread *thread, BasicType type, size_t length) {
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

        addToOopHolder(thread, oop);
        return oop;
    }

    ByteTypeArrayOop *OopManager::newByteArrayOop(VMThread *thread, size_t length) {
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(BasicType::T_BYTE);
        const auto oop = new ByteTypeArrayOop(klass, length);
        addToOopHolder(thread, oop);
        return oop;
    }

    ByteTypeArrayOop *OopManager::newByteArrayOop(VMThread *thread, size_t length, const u1 *initBuffer) {
        const auto oop = newByteArrayOop(thread, length);
        std::copy(initBuffer, initBuffer + length, oop->data.get());
        return oop;
    }

    CharTypeArrayOop *OopManager::newCharArrayOop(VMThread *thread, size_t length) {
        const auto klass = vm.bootstrapClassLoader->getTypeArrayClass(BasicType::T_CHAR);
        const auto oop = new CharTypeArrayOop(klass, length);
        addToOopHolder(thread, oop);
        return oop;
    }

    InstanceOop *OopManager::newBooleanOop(VMThread *thread, i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_BOOLEAN);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "Z", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newByteOop(VMThread *thread, i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_BYTE);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "B", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newCharOop(VMThread *thread, i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_CHARACTER);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "C", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newShortOop(VMThread *thread, i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_SHORT);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "S", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newIntegerOop(VMThread *thread, i4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INTEGER);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "I", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newFloatOop(VMThread *thread, f4 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_FLOAT);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "F", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newLongOop(VMThread *thread, i8 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_LONG);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "J", Slot(value));
        return oop;
    }

    InstanceOop *OopManager::newDoubleOop(VMThread *thread, f8 value) {
        const auto klass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_DOUBLE);
        const auto oop = newInstance(thread, klass);
        oop->setFieldValue("value", "D", Slot(value));
        return oop;
    }

    void OopManager::addToOopHolder(VMThread *thread, ref oop) {
        thread->oopHolder.addOop(oop);
        ++oopCount;
    }

    void traceOop(Oop * const oop, std::unordered_set<Oop *> &tracedOop) {
        if (oop == nullptr) {
            return;
        }
        if (tracedOop.contains(oop)) {
            return;
        }
        tracedOop.insert(oop);

        const auto oopType = oop->getType();
        switch (oopType) {
            case OopTypeEnum::INSTANCE_OOP:
                traceInstanceOopChild(CAST_INSTANCE_OOP(oop), tracedOop);
            break;

            case OopTypeEnum::OBJ_ARRAY_OOP:
                traceObjArrayOopChild(CAST_OBJ_ARRAY_OOP(oop), tracedOop);
            break;

            case OopTypeEnum::TYPE_ARRAY_OOP:
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
        const auto arrayLength = oop->getDataLength();
        if (arrayLength > 0) {
            for (size_t i = 0; i < arrayLength; ++i) {
                const auto element = oop->data[i];
                if (element != nullptr) {
                    traceOop(element, tracedOop);
                }
            }
        }
    }

    void collectOop(ref oop) {
        const auto klass = oop->getClass();
        if (klass->getType() == ClassTypeEnum::INSTANCE_CLASS) {
            const auto instanceClass = CAST_INSTANCE_CLASS(klass);
            const auto finalizeMethod = instanceClass->getMethodSelf("finalize", "()V", false);
            if (finalizeMethod != nullptr) {
                
            }
        }
    }

    void collectAll(VM &vm) {
        const auto &oopManager = vm.oopManager;
        auto &allocatedOop = oopManager->allocatedOop;

        for (auto &oop : allocatedOop) {
            delete oop;
        }
    }
    
}