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

    template<typename T>
    bool weakCompareAndSwap(T* src, T expected, T desired) {
        const auto atom = reinterpret_cast<std::atomic<T>*>(src);
        bool success = false;
        do {
            expected = atom->load(std::memory_order_relaxed);
            if (expected == *src) { // 额外检查，确保src值未被其他线程更改
                success = atom->compare_exchange_weak(expected, desired, std::memory_order_seq_cst);
                if (success) {
                    *src = desired; // 成功后同步回原指针
                }
            }
        } while (!success);
        return success;
    }

    template <typename T>
    bool strongCompareAndSwap(T *src, T expect, T val) {
        static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
        std::atomic<T>* atomSrc = reinterpret_cast<std::atomic<T>*>(src);
        T temp = expect;
        return std::atomic_compare_exchange_strong(atomSrc, &temp, val);
    }

    template <typename T>
    bool simpleCompareAndSwap(T *src, T expect, T val) {
        if (*src == expect) {
            *src = val;
            return true;
        }
        return false;
    }

    template <typename T>
    bool compareAndSwap(T *src, T expect, T val) {
        return strongCompareAndSwap(src, expect, val);
        //return simpleCompareAndSwap(src, expect, val);
    }

    template<typename T>
    T getObjectVolatileHelper(T *obj) {
        std::atomic<T>& atomicObj = *reinterpret_cast<std::atomic<T>*>(obj);
        return atomicObj.load(std::memory_order_acquire);
    }

    template<typename T>
    void putObjectVolatileHelper(T *obj, T newValue) {
        std::atomic<T>& atomicObj = *reinterpret_cast<std::atomic<T>*>(obj);
        atomicObj.store(newValue, std::memory_order_release);
    }

    void compareAndSwapAction(Frame &frame, Slot *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected, x));
    }

    void compareAndSwapAction(Frame &frame, ref *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected.refVal, x.refVal));
    }

    void compareAndSwapAction(Frame &frame, u1 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, CAST_U1(expected.i4Val), CAST_U1(x.i4Val)));
    }

    void compareAndSwapAction(Frame &frame, cchar_16 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, CAST_CCHAR_16(expected.i4Val), CAST_CCHAR_16(x.i4Val)));
    }

    void compareAndSwapAction(Frame &frame, i2 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, CAST_I2(expected.i4Val), CAST_I2(x.i4Val)));
    }

    void compareAndSwapAction(Frame &frame, i4 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected.i4Val, x.i4Val));
    }

    void compareAndSwapAction(Frame &frame, i8 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected.i8Val, x.i8Val));
    }

    void compareAndSwapAction(Frame &frame, f4 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected.f4Val, x.f4Val));
    }

    void compareAndSwapAction(Frame &frame, f8 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, expected.f8Val, x.f8Val));
    }

    void putVolatileAction(Frame &frame, Slot *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x);
    }

    void putVolatileAction(Frame &frame, ref *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x.refVal);
    }

    void putVolatileAction(Frame &frame, u1 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, CAST_U1(x.i4Val));
    }

    void putVolatileAction(Frame &frame, cchar_16 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, CAST_CCHAR_16(x.i4Val));
    }

    void putVolatileAction(Frame &frame, i2 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, CAST_I2(x.i4Val));
    }

    void putVolatileAction(Frame &frame, i4 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x.i4Val);
    }

    void putVolatileAction(Frame &frame, i8 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x.i8Val);
    }

    void putVolatileAction(Frame &frame, f4 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x.f4Val);
    }

    void putVolatileAction(Frame &frame, f8 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, x.f8Val);
    }

    void putAction(Frame &frame, Slot *obj) {
        const auto x = frame.getLocal(4);
        *obj = x;
    }

    void putAction(Frame &frame, ref *obj) {
        const auto x = frame.getLocal(4);
        *obj = x.refVal;
    }

    void putAction(Frame &frame, u1 *obj) {
        const auto x = frame.getLocal(4);
        *obj = CAST_U1(x.i4Val);
    }

    void putAction(Frame &frame, cchar_16 *obj) {
        const auto x = frame.getLocal(4);
        *obj = CAST_CCHAR_16(x.i4Val);
    }

    void putAction(Frame &frame, i2 *obj) {
        const auto x = frame.getLocal(4);
        *obj = CAST_I2(x.i4Val);
    }

    void putAction(Frame &frame, i4 *obj) {
        const auto x = frame.getLocal(4);
        *obj = x.i4Val;
    }

    void putAction(Frame &frame, i8 *obj) {
        const auto x = frame.getLocal(4);
        *obj = x.i8Val;
    }

    void putAction(Frame &frame, f4 *obj) {
        const auto x = frame.getLocal(4);
        *obj = x.f4Val;
    }

    void putAction(Frame &frame, f8 *obj) {
        const auto x = frame.getLocal(4);
        *obj = x.f8Val;
    }


    void getVolatileAction(Frame &frame, Slot *dataPtr, SlotTypeEnum type) {
        frame.returnSlot(getObjectVolatileHelper(dataPtr), type);
    }

    template<typename T>
    void getVolatileAction(Frame &frame, T *dataPtr, SlotTypeEnum type) {
        frame.returnSlot(Slot(getObjectVolatileHelper(dataPtr)), type);
    }

    void getAction(Frame &frame, Slot *dataPtr, SlotTypeEnum type) {
        frame.returnSlot(*dataPtr, type);
    }

    template<typename T>
    void getAction(Frame &frame, T *dataPtr, SlotTypeEnum type) {
        frame.returnSlot(Slot(*dataPtr), type);
    }

    template<typename T>
    void setMemoryAction(Frame &frame, T *dataPtr) {
        const auto bytes = CAST_SIZE_T(frame.getLocalI8(4));
        const auto value = CAST_U1(frame.getLocalI4(6));
        const auto rawPtr = reinterpret_cast<void *>(dataPtr);
        std::memset(rawPtr, value, bytes);
    }

    enum class UnsafeActionTypeEnum {
        COMPARE_AND_SWAP,
        GET_VOLATILE,
        PUT_VOLATILE,
        GET,
        PUT,
        SET_MEMORY,
    };

    template<typename T>
    void actionSwitch(Frame &frame, UnsafeActionTypeEnum actionType, T *dataPtr, SlotTypeEnum slotType) {
        switch (actionType) {
            case UnsafeActionTypeEnum::COMPARE_AND_SWAP: {
                const auto expected = frame.getLocal(4);
                const auto x = 
                    (slotType == SlotTypeEnum::I8 || slotType == SlotTypeEnum::F8) ?
                             frame.getLocal(6) : 
                             frame.getLocal(5);
                compareAndSwapAction(frame, dataPtr, expected, x);
                break;
            }

            case UnsafeActionTypeEnum::GET_VOLATILE: {
                getVolatileAction(frame, dataPtr, slotType);
                break;
            }

            case UnsafeActionTypeEnum::PUT_VOLATILE: {
                putVolatileAction(frame, dataPtr);
                break;
            }

            case UnsafeActionTypeEnum::GET: {
                getAction(frame, dataPtr, slotType);
                break;
            }

            case UnsafeActionTypeEnum::PUT: {
                putAction(frame, dataPtr);
                break;
            }

            case UnsafeActionTypeEnum::SET_MEMORY:
                setMemoryAction(frame, dataPtr);
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
        if (obj == nullptr) {
            if (actionType != UnsafeActionTypeEnum::SET_MEMORY) {
                panic("error object");
            }
            //offset就是address 转换成i4 *是为了适配泛型方法
            const auto dataPtr = reinterpret_cast<i4 *>(std::bit_cast<void *>(offset));
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        }
        
        if (isStaticFieldOffset(offset)) {
            offset = decodeStaticFieldOffset(offset);
            if (obj->klass->name != JAVA_LANG_CLASS_NAME) {
                panic("error object");
            }
            const auto mirrorClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(obj)->mirrorClass);
            auto dataPtr = mirrorClass->staticData.get();
            dataPtr += offset;
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        }

        if (obj->type == OopTypeEnum::InstanceOop) {
            const auto instanceObj = CAST_INSTANCE_OOP(obj);
            auto dataPtr = instanceObj->data.get();
            dataPtr += offset;
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        } else if (obj->type == OopTypeEnum::ObjArrayOop) {
            const auto arrayObj = CAST_OBJ_ARRAY_OOP(obj);
            auto dataPtr = arrayObj->data.get();
            dataPtr += offset;
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        } else {
            const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(obj->klass);
            const auto basicType = getBasicTypeByTypeArrayClassName(typeArrayClass->name);
            switch (basicType) {
                case BasicType::T_BOOLEAN:
                case BasicType::T_BYTE: {
                    const auto arrayObj = CAST_BYTE_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_CHAR: {
                    const auto arrayObj = CAST_CHAR_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_SHORT: {
                    const auto arrayObj = CAST_SHORT_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_INT: {
                    const auto arrayObj = CAST_INT_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_FLOAT: {
                    const auto arrayObj = CAST_FLOAT_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_LONG: {
                    const auto arrayObj = CAST_LONG_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_DOUBLE: {
                    const auto arrayObj = CAST_DOUBLE_TYPE_ARRAY_OOP(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                default:
                    panic("unsafCommon error");
            }
        }
    }

}

#endif