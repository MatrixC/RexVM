#include "frame_memory_handler.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "mirror_oop.hpp"
#include "memory.hpp"
#include "vm.hpp"
#include "string_pool.hpp"
#include "class_loader.hpp"
#include "constant_info.hpp"
#include "method_handle.hpp"
#include "composite_ptr.hpp"
#include "utils/descriptor_parser.hpp"
#include "method_handle.hpp"

namespace RexVM {

    FrameMemoryHandler::FrameMemoryHandler(Frame &frame) :
        frame(frame), vmThread(frame.thread), oopManager(*frame.vm.oopManager), stringPool(*frame.vm.stringPool), classLoader(*frame.getCurrentClassLoader()) {
            executeClassMemberCache.reserve(100);
    }

    InstanceOop *FrameMemoryHandler::newInstance(InstanceClass * klass) const {
        const auto oop = oopManager.newInstance(&vmThread, klass);
        frame.addCreateRef(oop);
        return oop;
    }

    MirOop *FrameMemoryHandler::newMirror(InstanceClass * klass, const voidPtr mirror, const MirrorObjectTypeEnum type) const {
        const auto oop = oopManager.newMirror(&vmThread, klass, mirror, type);
        frame.addCreateRef(oop);
        return oop;
    }

    ObjArrayOop *FrameMemoryHandler::newObjArrayOop(ObjArrayClass * klass, const size_t length) const {
        const auto oop = oopManager.newObjArrayOop(&vmThread, klass, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ObjArrayOop *FrameMemoryHandler::newObjectObjArrayOop(const size_t length) const {
        const auto oop = oopManager.newObjectObjArrayOop(&vmThread, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ObjArrayOop *FrameMemoryHandler::newClassObjArrayOop(const size_t length) const {
        const auto oop = oopManager.newClassObjArrayOop(&vmThread, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ObjArrayOop *FrameMemoryHandler::newStringObjArrayOop(const size_t length) const {
        const auto oop = oopManager.newStringObjArrayOop(&vmThread, length);
        frame.addCreateRef(oop);
        return oop;
    }

    TypeArrayOop *FrameMemoryHandler::newTypeArrayOop(const BasicType type, const size_t length) const {
        const auto oop = oopManager.newTypeArrayOop(&vmThread, type, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(const size_t length) const {
        const auto oop = oopManager.newByteArrayOop(&vmThread, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(const size_t length, const u1 *initBuffer) const {
        const auto oop = oopManager.newByteArrayOop(&vmThread, length, initBuffer);
        frame.addCreateRef(oop);
        return oop;
    }

    CharTypeArrayOop *FrameMemoryHandler::newCharArrayOop(const size_t length) const {
        const auto oop = oopManager.newCharArrayOop(&vmThread, length);
        frame.addCreateRef(oop);
        return oop;
    }

    ref FrameMemoryHandler::newMultiArrayOop(const u2 index, i4 *dimLength, const i2 dimCount) {
        const auto &constantPool = frame.constantPool;
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto oop = newMultiArrayOop(dimLength, dimCount, className, 0);
        frame.addCreateRef(oop);
        return oop;
    }

    ref FrameMemoryHandler::newMultiArrayOop(i4 *dimLength, const i2 dimCount, const cview name, const i4 currentDim) {
        const auto arrayLength = dimLength[currentDim];
        const auto currentArrayClass = frame.mem.getArrayClass(name);
        ArrayOop *arrayOop;
        if (currentArrayClass->type == ClassTypeEnum::TYPE_ARRAY_CLASS) {
            const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(currentArrayClass);
            arrayOop = frame.mem.newTypeArrayOop(typeArrayClass->elementType, arrayLength);
        } else {
            const auto objArrayClass = CAST_OBJ_ARRAY_CLASS(currentArrayClass);
            arrayOop = frame.mem.newObjArrayOop(objArrayClass, arrayLength);
        }

        if (currentDim == dimCount - 1) {
            return arrayOop;
        }

        const auto objArrayOop = CAST_OBJ_ARRAY_OOP(arrayOop);
        if (currentDim < dimCount - 1) {
            const auto childName = name.substr(1);
            for (auto i = 0; i < arrayLength; ++i) {
                objArrayOop->data[i] = newMultiArrayOop(dimLength, dimCount, childName, currentDim + 1);
            }
        }
        return objArrayOop;
    }

    InstanceOop *FrameMemoryHandler::newBooleanOop(const i4 value) const {
        const auto oop = oopManager.newBooleanOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newByteOop(const i4 value) const {
        const auto oop = oopManager.newByteOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newCharOop(const i4 value) const {
        const auto oop = oopManager.newCharOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newShortOop(const i4 value) const {
        const auto oop = oopManager.newShortOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newIntegerOop(const i4 value) const {
        const auto oop = oopManager.newIntegerOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newFloatOop(const f4 value) const {
        const auto oop = oopManager.newFloatOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newLongOop(const i8 value) const {
        const auto oop = oopManager.newLongOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::newDoubleOop(const f8 value) const {
        const auto oop = oopManager.newDoubleOop(&vmThread, value);
        frame.addCreateRef(oop);
        return oop;
    }

    InstanceOop *FrameMemoryHandler::getInternString(const cview str) const {
        const auto oop = stringPool.getInternString(&vmThread, str);
        frame.addCreateRef(oop);
        return oop;
    }

    Class *FrameMemoryHandler::getClass(const cview name) const {
        return classLoader.getClass(name);
    }

    InstanceClass *FrameMemoryHandler::getInstanceClass(const cview name) const {
        return classLoader.getInstanceClass(name);
    }

    ArrayClass *FrameMemoryHandler::getArrayClass(const cview name) const {
        return classLoader.getArrayClass(name);
    }

    TypeArrayClass *FrameMemoryHandler::getTypeArrayClass(const BasicType type) const {
        return classLoader.getTypeArrayClass(type);
    }
    
    ObjArrayClass *FrameMemoryHandler::getObjectArrayClass(const Class &klass) const {
        return classLoader.getObjectArrayClass(klass);
    }

    InstanceClass *FrameMemoryHandler::loadInstanceClass(const u1 *ptr, const size_t length, const bool notAnonymous) const {
        return classLoader.loadInstanceClass(ptr, length, notAnonymous);
    }

    InstanceClass *FrameMemoryHandler::getBasicJavaClass(const BasicJavaClassEnum classEnum) const {
        return classLoader.getBasicJavaClass(classEnum);
    }


    Field *FrameMemoryHandler::getRefField(u2 index, bool isStatic) {
        if (const auto member = executeClassMemberCache.try_get(CAST_U8(index)); member != nullptr) {
            return CAST_FIELD(*member);
        }
        const auto &klass = frame.klass;
        const auto fieldRef = klass.getRefField(index, isStatic);
        if (isStatic) {
            fieldRef->klass.clinit(frame);
            if (frame.markThrow) {
                return nullptr;
            }
        }
        executeClassMemberCache.emplace_unique(index, fieldRef);
        return fieldRef;
    }

    Method *FrameMemoryHandler::getRefMethod(u2 index, const bool isStatic) {
        if (const auto member = executeClassMemberCache.try_get(CAST_U8(index)); member != nullptr) {
            return CAST_METHOD(*member);
        }
        const auto &klass = frame.klass;
        const auto methodRef = klass.getRefMethod(index, isStatic);
        if (isStatic) {
            methodRef->klass.clinit(frame);
            if (frame.markThrow) {
                return nullptr;
            }
        }
        executeClassMemberCache.emplace_unique(index, methodRef);
        return methodRef;
    }

    Class *FrameMemoryHandler::getRefClass(u2 index) {
        if (const auto member = executeClassMemberCache.try_get(CAST_U8(index)); member != nullptr) {
            return CAST_CLASS(*member);
        }
        auto &klass = frame.klass;
        const auto className = getConstantStringFromPoolByIndexInfo(klass.constantPool, index);
        const auto refClass = klass.classLoader.getClass(className);
        executeClassMemberCache.emplace_unique(index, refClass);
        return refClass;
    }

    ExecuteVirtualMethodCache *FrameMemoryHandler::resolveInvokeVirtualIndex(u2 index, const bool checkMethodHandle) {
        if (const auto member = executeClassMemberCache.try_get(CAST_U8(index)); member != nullptr) {
            return static_cast<ExecuteVirtualMethodCache *>(*member);
        }
        auto cache = std::make_unique<ExecuteVirtualMethodCache>();
        auto cachePtr = cache.get();
        cacheVector.emplace_back(std::move(cache));
        executeClassMemberCache.emplace_unique(index, cachePtr);

        const auto &klass = frame.klass;
        const auto &constantPool = klass.constantPool;
        auto [className, methodName, methodDescriptor] = getConstantStringFromPoolByClassNameType(constantPool, index);
        cachePtr->methodName = methodName;
        cachePtr->methodDescriptor = methodDescriptor;

        if (checkMethodHandle && isMethodHandleInvoke(className, methodName)) {
            const auto invokeMethod =
                    getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INVOKE_METHOD_HANDLE)
                    ->getMethod(methodName, METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR, false);
            cachePtr->mhMethod = invokeMethod;
            cachePtr->mhMethodPopSize = getMethodParamSlotSizeFromDescriptor(methodDescriptor, false);
            return cachePtr;
        }

        cachePtr->paramSlotSize = getMethodParamSlotSizeFromDescriptor(methodDescriptor, false);
        return cachePtr;
     }

    Method *FrameMemoryHandler::linkVirtualMethod(
        const u2 index,
        const cview methodName,
        const cview methodDescriptor,
        InstanceClass *instanceClass
    ) {
        const Composite keyComposite(instanceClass, index);
        u8 key = keyComposite.composite;
        if (const auto member = executeClassMemberCache.try_get(key); member != nullptr) {
            return CAST_METHOD(*member);
        }

        if (instanceClass->isArray()) {
            //Only clone, getClass, toString method can be call
            const auto objectClass = frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT);
            const auto realInvokeMethod = objectClass->getMethod(methodName,methodDescriptor, false);
            if (realInvokeMethod == nullptr) {
                panic("array invoke error");
            }
            executeClassMemberCache.emplace_unique(key, realInvokeMethod);
            return realInvokeMethod;
        } else {
            for (auto k = instanceClass; k != nullptr; k = k->getSuperClass()) {
                const auto realInvokeMethod = k->getMethod(methodName, methodDescriptor, false);
                if (realInvokeMethod != nullptr && !realInvokeMethod->isAbstract()) {
                    executeClassMemberCache.emplace_unique(key, realInvokeMethod);
                    return realInvokeMethod;
                }
            }
        }
        panic("method not found");
        return nullptr;
     }

    InstanceOop *FrameMemoryHandler::invokeDynamic(const u2 invokeDynamicIdx) const {
        const auto oop = RexVM::invokeDynamic(frame, invokeDynamicIdx);
        frame.addCreateRef(oop);
        return oop;
    }



}