#include "frame_memory_handler.hpp"
#include "basic_type.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "thread.hpp"
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
    }

    InstanceOop *FrameMemoryHandler::newInstance(InstanceClass * klass) {
        return oopManager.newInstance(&vmThread, klass);
    }

    MirOop *FrameMemoryHandler::newMirror(InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type) {
        return oopManager.newMirror(&vmThread, klass, mirror, type);
    }

    ObjArrayOop *FrameMemoryHandler::newObjArrayOop(ObjArrayClass * klass, size_t length) {
        return oopManager.newObjArrayOop(&vmThread, klass, length);
    }

    ObjArrayOop *FrameMemoryHandler::newObjectObjArrayOop(size_t length) {
        return oopManager.newObjectObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newClassObjArrayOop(size_t length) {
        return oopManager.newClassObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newStringObjArrayOop(size_t length) {
        return oopManager.newStringObjArrayOop(&vmThread, length);
    }

    TypeArrayOop *FrameMemoryHandler::newTypeArrayOop(BasicType type, size_t length) {
        return oopManager.newTypeArrayOop(&vmThread, type, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(size_t length) {
        return oopManager.newByteArrayOop(&vmThread, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(size_t length, const u1 *initBuffer) {
        return oopManager.newByteArrayOop(&vmThread, length, initBuffer);
    }

    CharTypeArrayOop *FrameMemoryHandler::newCharArrayOop(size_t length) {
        return oopManager.newCharArrayOop(&vmThread, length);
    }

    InstanceOop *FrameMemoryHandler::newBooleanOop(i4 value) {
        return oopManager.newBooleanOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newByteOop(i4 value) {
        return oopManager.newByteOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newCharOop(i4 value) {
        return oopManager.newCharOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newShortOop(i4 value) {
        return oopManager.newShortOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newIntegerOop(i4 value) {
        return oopManager.newIntegerOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newFloatOop(f4 value) {
        return oopManager.newFloatOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newLongOop(i8 value) {
        return oopManager.newLongOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newDoubleOop(f8 value) {
        return oopManager.newDoubleOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::getInternString(cview str) {
        return stringPool.getInternString(&vmThread, str);
    }

    Class *FrameMemoryHandler::getClass(const cstring &name) {
        return classLoader.getClass(name);
    }

    Class *FrameMemoryHandler::getClass(cview name) {
        return classLoader.getClass(name);
    }

    // InstanceClass *FrameMemoryHandler::getInstanceClass(const cstring &name) {
    //     return classLoader.getInstanceClass(name);
    // }

    // ArrayClass *FrameMemoryHandler::getArrayClass(const cstring &name) {
    //     return classLoader.getArrayClass(name);
    // }

    InstanceClass *FrameMemoryHandler::getInstanceClass(cview name) {
        return classLoader.getInstanceClass(name);
    }

    ArrayClass *FrameMemoryHandler::getArrayClass(cview name) {
        return classLoader.getArrayClass(name);
    }

    TypeArrayClass *FrameMemoryHandler::getTypeArrayClass(BasicType type) {
        return classLoader.getTypeArrayClass(type);
    }
    
    ObjArrayClass *FrameMemoryHandler::getObjectArrayClass(const Class &klass) {
        return classLoader.getObjectArrayClass(klass);
    }

    InstanceClass *FrameMemoryHandler::loadInstanceClass(u1 *ptr, size_t length, bool notAnonymous) {
        return classLoader.loadInstanceClass(ptr, length, notAnonymous);
    }

    InstanceClass *FrameMemoryHandler::getBasicJavaClass(BasicJavaClassEnum classEnum) const {
        return classLoader.getBasicJavaClass(classEnum);
    }


    Field *FrameMemoryHandler::getRefField(u2 index, bool isStatic) {
        if (const auto iter = executeClassMemberCache.find(CAST_U8(index)); iter != executeClassMemberCache.end()) {
            return CAST_FIELD(iter->second);
        }
        auto &klass = frame.klass;
        const auto fieldRef = klass.getRefField(index, isStatic);
        if (isStatic) {
            fieldRef->klass.clinit(frame);
        }
        executeClassMemberCache.emplace(index, fieldRef);
        return fieldRef;
    }

    Method *FrameMemoryHandler::getRefMethod(u2 index, bool isStatic) {
        if (const auto iter = executeClassMemberCache.find(CAST_U8(index)); iter != executeClassMemberCache.end()) {
            return CAST_METHOD(iter->second);
        }
        auto &klass = frame.klass;
        const auto methodRef = klass.getRefMethod(index, isStatic);
        if (isStatic) {
            methodRef->klass.clinit(frame);
        }
        executeClassMemberCache.emplace(index, methodRef);
        return methodRef;
    }

    Class *FrameMemoryHandler::getRefClass(u2 index) {
        if (const auto iter = executeClassMemberCache.find(CAST_U8(index)); iter != executeClassMemberCache.end()) {
            return CAST_CLASS(iter->second);
        }
        auto &klass = frame.klass;
        const auto className = getConstantStringFromPoolByIndexInfo(klass.constantPool, index);
        const auto refClass = klass.classLoader.getClass(className);
        executeClassMemberCache.emplace(index, refClass);
        return refClass;
    }

    ExecuteVirutalMethodCache *FrameMemoryHandler::resolveInvokeVirtualIndex(u2 index, bool checkMethodHandle) {
        ExecuteVirutalMethodCache *cachePtr = nullptr;
        const auto iter = executeClassMemberCache.find(index);
        if (iter != executeClassMemberCache.end()) {
            cachePtr = static_cast<ExecuteVirutalMethodCache *>(iter->second);
            return cachePtr;
        } else {
            auto cache = std::make_unique<ExecuteVirutalMethodCache>();
            cachePtr = cache.get();
            cacheVector.emplace_back(std::move(cache));
            executeClassMemberCache.emplace(index, cachePtr);
        }

        auto &klass = frame.klass;
        const auto &constantPool = klass.constantPool;
        auto [className, methodName, methodDescriptor] = getConstantStringFromPoolByClassNameType(constantPool, index);
        cachePtr->methodName = methodName;
        cachePtr->methodDescriptor = methodDescriptor;
        
        if (checkMethodHandle) {
            const auto mhInvoke = isMethodHandleInvoke(className, methodName);
            if (mhInvoke) {
                const auto invokeMethod = 
                    getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INVOKE_METHOD_HANDLE)
                        ->getMethod(methodName, METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR, false);
                cachePtr->mhMethod = invokeMethod;
                //1第一个参数为MethodHandle Object
                cachePtr->mhMethodPopSize = getMethodParamSlotSizeFromDescriptor(methodDescriptor, false);
                //此次调用为MethodHandle调用 直接返回
                return cachePtr;
            }
        }

        cachePtr->paramSlotSize = getMethodParamSlotSizeFromDescriptor(methodDescriptor, false);
        return cachePtr;
     }

     Method *FrameMemoryHandler::linkVirtualMethod(u2 index, ExecuteVirutalMethodCache *cache, InstanceClass *instanceClass) {
        Composite<InstanceClass *, u2> keyComposite(instanceClass, index);
        u8 key = keyComposite.composite;
        const auto iter = executeClassMemberCache.find(key);
        if (iter != executeClassMemberCache.end()) {
            return CAST_METHOD(iter->second);
        }

        if (instanceClass->isArray()) {
            //Only clone and getClass method can be call
            const auto objectClass = frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT);
            const auto realInvokeMethod = objectClass->getMethod(cache->methodName, cache->methodDescriptor, false);
            if (realInvokeMethod == nullptr) {
                panic("array invoke error");
            }
            executeClassMemberCache.emplace(key, realInvokeMethod);
            return realInvokeMethod;
        } else {
            for (auto k = instanceClass; k != nullptr; k = k->getSuperClass()) {
                const auto realInvokeMethod = k->getMethod(cache->methodName, cache->methodDescriptor, false);
                if (realInvokeMethod != nullptr && !realInvokeMethod->isAbstract()) {
                    executeClassMemberCache.emplace(key, realInvokeMethod);
                    return realInvokeMethod;
                }
            }
        }
        panic("method not found");
        return nullptr;
     }


}