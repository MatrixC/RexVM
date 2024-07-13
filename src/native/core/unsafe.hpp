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
        if (mirrorClass->type == ClassTypeEnum::InstanceClass) {
            (CAST_INSTANCE_CLASS(mirrorClass))->clinit(frame);
        }
    }

    //native boolean shouldBeInitialized(Class<?> c);
    void shouldBeInitialized(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorClass = mirrorOop->mirrorClass;
        if (mirrorClass->type == ClassTypeEnum::InstanceClass) {
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
        frame.returnI4(1);
    }

    void addressSize(Frame &frame) {
        frame.returnI4(sizeof(ref));
    }

    void objectFieldOffset(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("slot", "I").i4Val;
        frame.returnI8(value);
    }

    //native long staticFieldOffset(Field f);
    void staticFieldOffset(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto mirrorOop = CAST_MIRROR_OOP(fieldMirror->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto value = fieldMirror->getFieldValue("slot", "I").i4Val;
        const auto encodeValue = (value + 1) * -1;
        frame.returnI8(mirrorClass->instanceSlotCount + value);
    }

    //native Object staticFieldBase(Field f);
    void staticFieldBase(Frame &frame) {
        const auto fieldMirror = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("clazz", "Ljava/lang/Class;").refVal;
        frame.returnRef(value);
    }

    //native boolean compareAndSwapObject(Object o, long offset, Object expected, Object x);
    void compareAndSwapObject(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::REF);
    }

    //native boolean compareAndSwapInt(Object o, long offset, int expected, int x);
    void compareAndSwapInt(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::I4);
    }

    //native boolean compareAndSwapLong(Object o, long offset, long expected, long x);
    void compareAndSwapLong(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::COMPARE_AND_SWAP, SlotTypeEnum::I8);
    }

    void getObjectVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::REF);
    }

    void putObjectVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::REF);
    }

    void getVolatileI4(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::I4);
    }

    void putVolatileI4(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::I4);
    }

    void getLongVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::I8);
    }

    void putLongVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::I8);
    }

    void getFloatVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::F4);
    }

    void putFloatVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::F4);
    }

    void getDoubleVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET_VOLATILE, SlotTypeEnum::F8);
    }

    void putDoubleVolatile(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT_VOLATILE, SlotTypeEnum::F8);
    }

    void getObject(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::REF);
    }

    void putObject(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::REF);
    }

    void getI4(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::I4);
    }

    void putI4(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::I4);
    }

    void getLong(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::I8);
    }

    void putLong(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::I8);
    }

    void getFloat(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::F4);
    }

    void putFloat(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::F4);
    }

    void getDouble(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::GET, SlotTypeEnum::F8);
    }

    void putDouble(Frame &frame) {
        unsafCommon(frame, UnsafeActionTypeEnum::PUT, SlotTypeEnum::F8);
    }

    //C heap function
    void allocateMemory(Frame &frame) {
        const auto length = frame.getLocalI8(1);
        const auto ptr = std::malloc(length);
        frame.returnI8(std::bit_cast<i8>(ptr));
    }

    void freeMemory(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<void *>(addressLong);
        std::free(address);
    }

    void reallocateMemory(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto size = CAST_SIZE_T(frame.getLocalI8(3));
        const auto address = std::bit_cast<void *>(addressLong);
        frame.returnI8(std::bit_cast<i8>(std::realloc(address, size)));
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




    




}

#endif