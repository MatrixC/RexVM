#include "frame_memory_handler.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "memory.hpp"
#include "vm.hpp"
#include "string_pool.hpp"
#include "class_loader.hpp"
#include "constant_info.hpp"
#include "method_handle.hpp"
#include "composite_ptr.hpp"
#include "utils/descriptor_parser.hpp"

namespace RexVM {

    FrameMemoryHandler::FrameMemoryHandler(Frame &frame) :
        frame(frame), vmThread(frame.thread), oopManager(*frame.vm.oopManager), stringPool(*frame.vm.stringPool), classLoader(*frame.getCurrentClassLoader()) {
            executeClassMemberCache.reserve(100);
    }

    InstanceOop *FrameMemoryHandler::newInstance(InstanceClass * klass) const {
        return oopManager.newInstance(&vmThread, klass);
    }

    MirOop *FrameMemoryHandler::newMirror(InstanceClass * klass, const voidPtr mirror, const MirrorObjectTypeEnum type) const {
        return oopManager.newMirror(&vmThread, klass, mirror, type);
    }

    ObjArrayOop *FrameMemoryHandler::newObjArrayOop(ObjArrayClass * klass, const size_t length) const {
        return oopManager.newObjArrayOop(&vmThread, klass, length);
    }

    ObjArrayOop *FrameMemoryHandler::newObjectObjArrayOop(const size_t length) const {
        return oopManager.newObjectObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newClassObjArrayOop(const size_t length) const {
        return oopManager.newClassObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newStringObjArrayOop(const size_t length) const {
        return oopManager.newStringObjArrayOop(&vmThread, length);
    }

    TypeArrayOop *FrameMemoryHandler::newTypeArrayOop(const BasicType type, const size_t length) const {
        return oopManager.newTypeArrayOop(&vmThread, type, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(const size_t length) const {
        return oopManager.newByteArrayOop(&vmThread, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(const size_t length, const u1 *initBuffer) const {
        return oopManager.newByteArrayOop(&vmThread, length, initBuffer);
    }

    CharTypeArrayOop *FrameMemoryHandler::newCharArrayOop(const size_t length) const {
        return oopManager.newCharArrayOop(&vmThread, length);
    }

    ref FrameMemoryHandler::newMultiArrayOop(const u2 index, i4 *dimLength, const i2 dimCount) {
        const auto &constantPool = frame.constantPool;
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        return newMultiArrayOop(dimLength, dimCount, className, 0);
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
        return oopManager.newBooleanOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newByteOop(const i4 value) const {
        return oopManager.newByteOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newCharOop(const i4 value) const {
        return oopManager.newCharOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newShortOop(const i4 value) const {
        return oopManager.newShortOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newIntegerOop(const i4 value) const {
        return oopManager.newIntegerOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newFloatOop(const f4 value) const {
        return oopManager.newFloatOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newLongOop(const i8 value) const {
        return oopManager.newLongOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newDoubleOop(const f8 value) const {
        return oopManager.newDoubleOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::getInternString(const cview str) const {
        return stringPool.getInternString(&vmThread, str);
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


}