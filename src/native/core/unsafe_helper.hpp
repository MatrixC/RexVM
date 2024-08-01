#ifndef NATIVE_CORE_UNSAFE_HELPER_HPP
#define NATIVE_CORE_UNSAFE_HELPER_HPP
#include <atomic>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../class.hpp"
#include "../../oop.hpp"
#include "../../basic_type.hpp"

namespace RexVM::Native::Core {

    bool compareAndSwapAction(void *src, Slot expect, Slot val, SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4: {
                const auto atomSrc = reinterpret_cast<std::atomic<i4>*>(src);
                return std::atomic_compare_exchange_strong(atomSrc, &expect.i4Val, val.i4Val);
            }
            case SlotTypeEnum::I8: {
                const auto atomSrc = reinterpret_cast<std::atomic<i8>*>(src);
                return std::atomic_compare_exchange_strong(atomSrc, &expect.i8Val, val.i8Val);
            }
            case SlotTypeEnum::F4: {
                const auto atomSrc = reinterpret_cast<std::atomic<f4>*>(src);
                return std::atomic_compare_exchange_strong(atomSrc, &expect.f4Val, val.f4Val);
            }
            case SlotTypeEnum::F8: {
                const auto atomSrc = reinterpret_cast<std::atomic<f8>*>(src);
                return std::atomic_compare_exchange_strong(atomSrc, &expect.f8Val, val.f8Val);
            }
            case SlotTypeEnum::REF: {
                const auto atomSrc = reinterpret_cast<std::atomic<ref>*>(src);
                return std::atomic_compare_exchange_strong(atomSrc, &expect.refVal, val.refVal);
            }
            default:
                panic("error slot type");
        } 
        return false;
    }

    void compareAndSwapAction(Frame &frame, void *dataPtr, Slot expected, Slot x, SlotTypeEnum slotType) {
        frame.returnBoolean(compareAndSwapAction(dataPtr, expected, x, slotType));
    }

    void putVolatileAction(Frame &frame, void *dataPtr, SlotTypeEnum slotType) {
        const auto x = frame.getLocal(4);
        switch (slotType) {
            case SlotTypeEnum::I4:
                (reinterpret_cast<std::atomic<i4>*>(dataPtr))->store(x.i4Val, std::memory_order_release);
                break;
            case SlotTypeEnum::I8:
                (reinterpret_cast<std::atomic<i8>*>(dataPtr))->store(x.i8Val, std::memory_order_release);
                break;
            case SlotTypeEnum::F4:
                (reinterpret_cast<std::atomic<f4>*>(dataPtr))->store(x.f4Val, std::memory_order_release);
                break;
            case SlotTypeEnum::F8:
                (reinterpret_cast<std::atomic<f8>*>(dataPtr))->store(x.f8Val, std::memory_order_release);
                break;
            case SlotTypeEnum::REF:
                (reinterpret_cast<std::atomic<ref>*>(dataPtr))->store(x.refVal, std::memory_order_release);
                break;
            default:
                panic("error slot type");
        }
    }

    void putAction(Frame &frame, void *dataPtr, SlotTypeEnum slotType) {
        const auto x = frame.getLocal(4);
        switch (slotType) {
            case SlotTypeEnum::I4:
                std::memcpy(dataPtr, &x.i4Val, sizeof(x.i4Val));
                break;
            case SlotTypeEnum::I8:
                std::memcpy(dataPtr, &x.i8Val, sizeof(x.i8Val));
                break;
            case SlotTypeEnum::F4:
                std::memcpy(dataPtr, &x.f4Val, sizeof(x.f4Val));
                break;
            case SlotTypeEnum::F8:
                std::memcpy(dataPtr, &x.f8Val, sizeof(x.f8Val));
                break;
            case SlotTypeEnum::REF:
                std::memcpy(dataPtr, &x.refVal, sizeof(x.refVal));
                break;
            default:
                panic("error slot type");
        }
    }

    void getVolatileAction(Frame &frame, void *dataPtr, SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
                frame.returnSlot(Slot((reinterpret_cast<std::atomic<i4>*>(dataPtr))->load(std::memory_order_acquire)), slotType);
                break;
            case SlotTypeEnum::I8:
                frame.returnSlot(Slot((reinterpret_cast<std::atomic<i8>*>(dataPtr))->load(std::memory_order_acquire)), slotType);
                break;
            case SlotTypeEnum::F4:
                frame.returnSlot(Slot((reinterpret_cast<std::atomic<f4>*>(dataPtr))->load(std::memory_order_acquire)), slotType);
                break;
            case SlotTypeEnum::F8:
                frame.returnSlot(Slot((reinterpret_cast<std::atomic<f8>*>(dataPtr))->load(std::memory_order_acquire)), slotType);
                break;
            case SlotTypeEnum::REF:
                frame.returnSlot(Slot((reinterpret_cast<std::atomic<ref>*>(dataPtr))->load(std::memory_order_acquire)), slotType);
                break;
            default:
                panic("error slot type");
        }
    }

    void getAction(Frame &frame, void *dataPtr, SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
                frame.returnSlot(Slot(* CAST_I4_PTR(dataPtr)), slotType);
                break;
            case SlotTypeEnum::I8:
                frame.returnSlot(Slot(* CAST_I8_PTR(dataPtr)), slotType);
                break;
            case SlotTypeEnum::F4:
                frame.returnSlot(Slot(* CAST_F4_PTR(dataPtr)), slotType);
                break;
            case SlotTypeEnum::F8:
                frame.returnSlot(Slot(* CAST_F8_PTR(dataPtr)), slotType);
                break;
            case SlotTypeEnum::REF:
                frame.returnSlot(Slot(* CAST_REF_PTR(dataPtr)), slotType);
                break;
            default:
                panic("error slot type");
        }
    }

    void setMemoryAction(Frame &frame, void *dataPtr) {
        const auto bytes = CAST_SIZE_T(frame.getLocalI8(4));
        const auto value = CAST_U1(frame.getLocalI4(6));
        std::memset(dataPtr, value, bytes);
    }

    enum class UnsafeActionTypeEnum {
        COMPARE_AND_SWAP,
        GET_VOLATILE,
        PUT_VOLATILE,
        GET,
        PUT,
        SET_MEMORY,
    };

    void actionSwitch(Frame &frame, UnsafeActionTypeEnum actionType, void *dataPtr, SlotTypeEnum slotType) {
        switch (actionType) {
            case UnsafeActionTypeEnum::COMPARE_AND_SWAP: {
                const auto expected = frame.getLocal(4);
                const auto x = 
                    (slotType == SlotTypeEnum::I8 || slotType == SlotTypeEnum::F8) ?
                             frame.getLocal(6) : 
                             frame.getLocal(5);
                compareAndSwapAction(frame, (void *)dataPtr, expected, x, slotType);
                break;
            }
            case UnsafeActionTypeEnum::GET_VOLATILE: {
                getVolatileAction(frame, (void *)dataPtr, slotType);
                break;
            }
            case UnsafeActionTypeEnum::PUT_VOLATILE: {
                putVolatileAction(frame, (void *)dataPtr, slotType);
                break;
            }
            case UnsafeActionTypeEnum::GET: {
                getAction(frame, (void *)dataPtr, slotType);
                break;
            }
            case UnsafeActionTypeEnum::PUT: {
                putAction(frame, (void *)dataPtr, slotType);
                break;
            }
            case UnsafeActionTypeEnum::SET_MEMORY:
                setMemoryAction(frame, (void *)dataPtr);
                break;
        }
    }
    
    i8 encodeStaticFieldOffset(i8 value) {
        return (value + 1) * -1;
    }

    i8 decodeStaticFieldOffset(i8 value) {
        return (value * -1) - 1;
    }

    bool isStaticFieldOffset(i8 value) {
        return value < 0;
    }

    void unsafCommon(Frame &frame, UnsafeActionTypeEnum actionType, SlotTypeEnum slotType) {
        //obj 有两种情况, 一个正常类的实例或者一个Class对象: 来源于staticFieldBase方法
        //如果要读取一个类的static字段, 则obj会传入类对应的Class对象
        //为了区分普通offset和static字段的offset, 在生成offset的时候做了一些区分, 静态offset用负数表示
        auto obj = frame.getLocalRef(1);
        auto offset = frame.getLocalI8(2);
        u1 *dataPtr;
        if (obj == nullptr) {
            if (actionType != UnsafeActionTypeEnum::SET_MEMORY) {
                panic("error object");
            }
            dataPtr = CAST_U1_PTR(std::bit_cast<void *>(offset));
        } else if (isStaticFieldOffset(offset)) {
            offset = decodeStaticFieldOffset(offset);
            if (obj->klass->name != JAVA_LANG_CLASS_NAME) {
                panic("error object");
            }
            const auto mirrorClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(obj)->mirrorClass);
            dataPtr = CAST_U1_PTR(mirrorClass->staticData.get());
        } else if (obj->type == OopTypeEnum::INSTANCE_OOP) {
            const auto instanceObj = CAST_INSTANCE_OOP(obj);
            dataPtr = CAST_U1_PTR(instanceObj->data.get());
        } else if (obj->type == OopTypeEnum::OBJ_ARRAY_OOP) {
            const auto arrayObj = CAST_OBJ_ARRAY_OOP(obj);
            dataPtr = CAST_U1_PTR(arrayObj->data.get());
        } else {
            const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(obj->klass);
            const auto basicType = getBasicTypeByTypeArrayClassName(typeArrayClass->name);
            switch (basicType) {
                case BasicType::T_BOOLEAN:
                case BasicType::T_BYTE:
                    dataPtr = CAST_U1_PTR(CAST_BYTE_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_CHAR:
                    dataPtr = CAST_U1_PTR(CAST_CHAR_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_SHORT:
                    dataPtr = CAST_U1_PTR(CAST_SHORT_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_INT:
                    dataPtr = CAST_U1_PTR(CAST_INT_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_FLOAT:
                    dataPtr = CAST_U1_PTR(CAST_FLOAT_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_LONG:
                    dataPtr = CAST_U1_PTR(CAST_LONG_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                case BasicType::T_DOUBLE:
                    dataPtr = CAST_U1_PTR(CAST_DOUBLE_TYPE_ARRAY_OOP(obj)->data.get());
                    break;
                default:
                    panic("unsafCommon error");
            }
        }
        dataPtr += offset;
        actionSwitch(frame, actionType, CAST_VOID_PTR(dataPtr), slotType);
    }

}

#endif