#ifndef NATIVE_CORE_UNSAFE_HPP
#define NATIVE_CORE_UNSAFE_HPP
#include <atomic>
#include "unsafe_helper.hpp"
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../class.hpp"
#include "../../oop.hpp"
#include "../../basic_type.hpp"

namespace RexVM::Native::Core {

    //native void ensureClassInitialized(Class<?> c);
    void ensureClassInitialized(Frame &frame) {
        const auto mirrorClassOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorClass = mirrorClassOop->mirrorClass;
        if (mirrorClass->type == ClassTypeEnum::INSTANCE_CLASS) {
            (CAST_INSTANCE_CLASS(mirrorClass))->clinit(frame);
        }
    }

    //native boolean shouldBeInitialized(Class<?> c);
    void shouldBeInitialized(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorClass = mirrorOop->mirrorClass;
        if (mirrorClass->type == ClassTypeEnum::INSTANCE_CLASS) {
            const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
            frame.returnBoolean(instanceMirrorClass->notInitialize());
            return;
        }
        frame.returnBoolean(false);
    }

    void arrayBaseOffset(Frame &frame) {
        frame.returnI4(0);
    }

    void arrayIndexScale(Frame &frame) {
        const auto arrayClassOop = frame.getLocalRef(1);
        if (arrayClassOop == nullptr) {
            throwNullPointException(frame);
            return;
        }
        if (arrayClassOop->klass->name != JAVA_LANG_CLASS_NAME) {
            panic("not class");
        }
        const auto arrayClass = GET_MIRROR_INSTANCE_CLASS(arrayClassOop);
        if (!arrayClass->isArray()) {
            panic("is not array class");
        }
        if (arrayClass->type == ClassTypeEnum::OBJ_ARRAY_CLASS) {
            frame.returnI4(sizeof(ref));
            return;
        }
        const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(arrayClass);
        const auto type = typeArrayClass->elementType;
        frame.returnI4(CAST_I4(getElementSizeByBasicType(type)));
    }

    void addressSize(Frame &frame) {
        frame.returnI4(sizeof(ref));
    }

    void objectFieldOffset(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("slot", "I").i4Val * 8;
        frame.returnI8(value);
    }

    //native long staticFieldOffset(Field f);
    void staticFieldOffset(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("slot", "I").i4Val * 8;
        //见unsafeCommon
        frame.returnI8(encodeStaticFieldOffset(value));
    }

    //native Object staticFieldBase(Field f);
    void staticFieldBase(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("clazz", "Ljava/lang/Class;").refVal;
        frame.returnRef(value);
    }

    //native boolean compareAndSwapObject(Object o, long offset, Object expected, Object x);
    void compareAndSwapObject(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::REF);
    }

    //native boolean compareAndSwapInt(Object o, long offset, int expected, int x);
    void compareAndSwapInt(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::I4);
    }

    //native boolean compareAndSwapLong(Object o, long offset, long expected, long x);
    void compareAndSwapLong(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::I8);
    }

    void getObjectVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::REF);
    }

    void putObjectVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::REF);
    }

    void getVolatileI4(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::I4);
    }

    void putVolatileI4(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::I4);
    }

    void getLongVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::I8);
    }

    void putLongVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::I8);
    }

    void getFloatVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::F4);
    }

    void putFloatVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::F4);
    }

    void getDoubleVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::F8);
    }

    void putDoubleVolatile(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::F8);
    }

    void getObject(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::REF);
    }

    void putObject(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::REF);
    }

    void getI4(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::I4);
    }

    void putI4(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::I4);
    }

    void getLong(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::I8);
    }

    void putLong(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::I8);
    }

    void getFloat(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::F4);
    }

    void putFloat(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::F4);
    }

    void getDouble(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::F8);
    }

    void putDouble(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::F8);
    }

    //C heap function
    void cheapAllocateMemory(Frame &frame) {
        const auto length = frame.getLocalI8(1);
        const auto ptr = std::malloc(length);
        frame.returnI8(std::bit_cast<i8>(ptr));
    }

    void cheapFreeMemory(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<voidPtr>(addressLong);
        std::free(address);
    }

    void cheapReallocateMemory(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto size = CAST_SIZE_T(frame.getLocalI8(3));
        const auto address = std::bit_cast<voidPtr>(addressLong);
        frame.returnI8(std::bit_cast<i8>(std::realloc(address, size)));
    }

    void cheapSetMemory(Frame &frame) {
        unsafeCommon(frame, UnsafeActionTypeEnum::SET_MEMORY, SlotTypeEnum::REF);
    }

    void cheapGetByte(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<i1 *>(addressLong);
        frame.returnI4(*address);
    }

    void cheapPutByte(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = CAST_I1(frame.getLocalI4(3));
        const auto address = std::bit_cast<i1 *>(addressLong);
        *address = value;
    }

    void cheapGetShort(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<i2 *>(addressLong);
        frame.returnI4(*address);
    }

    void cheapPutShort(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = CAST_I2(frame.getLocalI4(3));
        const auto address = std::bit_cast<i2 *>(addressLong);
        *address = value;
    }

    void cheapGetChar16(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<cchar_16 *>(addressLong);
        frame.returnI4(*address);
    }

    void cheapPutChar16(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = CAST_CCHAR_16(frame.getLocalI4(3));
        const auto address = std::bit_cast<cchar_16 *>(addressLong);
        *address = value;
    }

    void cheapGetI4(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<i4 *>(addressLong);
        frame.returnI4(*address);
    }

    void cheapPutI4(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = frame.getLocalI4(3);
        const auto address = std::bit_cast<i4 *>(addressLong);
        *address = value;
    }

    void cheapGetI8(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<i8 *>(addressLong);
        frame.returnI8(*address);
    }

    void cheapPutI8(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = frame.getLocalI8(3);
        const auto address = std::bit_cast<i8 *>(addressLong);
        *address = value;
    }

    void cheapGetF4(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<f4 *>(addressLong);
        frame.returnF4(*address);
    } 

    void cheapPutF4(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = frame.getLocalF4(3);
        const auto address = std::bit_cast<f4 *>(addressLong);
        *address = value;
    }

    void cheapGetF8(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<f8 *>(addressLong);
        frame.returnF8(*address);
    } 

    void cheapPutF8(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = frame.getLocalF8(3);
        const auto address = std::bit_cast<f8 *>(addressLong);
        *address = value;
    }

    void pageSize(Frame &frame) {
        const auto pageSize = CAST_I4(getSystemPageSize());
        frame.returnI4(pageSize);
    }

    //native Object allocateInstance(Class<?> cls) throws InstantiationException;
    void allocateInstance(Frame &frame) {
        const auto mirrorClass = GET_MIRROR_INSTANCE_CLASS(frame.getLocalRef(1));
        const auto &oopManager = frame.vm.oopManager;
        const auto instance = oopManager->newInstance(mirrorClass);
        frame.returnRef(instance);
    }

    //native void throwException(Throwable ee);
    void throwException(Frame &frame) {
        frame.throwException(CAST_INSTANCE_OOP(frame.getLocalRef(1)));
    }

    void loadFence(Frame &frame) {
        std::atomic_thread_fence(std::memory_order_acquire);
    }

    void storeFence(Frame &frame) {
        std::atomic_thread_fence(std::memory_order_release);
    }

    void fullFence(Frame &frame) {
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }
}

#endif