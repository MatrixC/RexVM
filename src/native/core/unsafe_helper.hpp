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
        auto* atomSrc = reinterpret_cast<std::atomic<T>*>(src);
        return std::atomic_compare_exchange_strong(atomSrc, &expect, val);
    }

    template <typename T>
    bool compareAndSwap(T *src, T expect, T val) {
        return weakCompareAndSwap(src, expect, val);
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
        frame.returnBoolean(compareAndSwap(dataPtr, static_cast<u1>(expected.i4Val), static_cast<u1>(x.i4Val)));
    }

    void compareAndSwapAction(Frame &frame, cchar_16 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, static_cast<cchar_16>(expected.i4Val), static_cast<cchar_16>(x.i4Val)));
    }

    void compareAndSwapAction(Frame &frame, i2 *dataPtr, Slot expected, Slot x) {
        frame.returnBoolean(compareAndSwap(dataPtr, static_cast<i2>(expected.i4Val), static_cast<i2>(x.i4Val)));
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
        putObjectVolatileHelper(obj, static_cast<u1>(x.i4Val));
    }

    void putVolatileAction(Frame &frame, cchar_16 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, static_cast<cchar_16>(x.i4Val));
    }

    void putVolatileAction(Frame &frame, i2 *obj) {
        const auto x = frame.getLocal(4);
        putObjectVolatileHelper(obj, static_cast<i2>(x.i4Val));
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
        *obj = static_cast<u1>(x.i4Val);
    }

    void putAction(Frame &frame, cchar_16 *obj) {
        const auto x = frame.getLocal(4);
        *obj = static_cast<cchar_16>(x.i4Val);
    }

    void putAction(Frame &frame, i2 *obj) {
        const auto x = frame.getLocal(4);
        *obj = static_cast<i2>(x.i4Val);
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

    enum class UnsafeActionTypeEnum {
        COMPARE_AND_SWAP,
        GET_VOLATILE,
        PUT_VOLATILE,
        GET,
        PUT,
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
        }
    }

    void unsafCommon(Frame &frame, UnsafeActionTypeEnum actionType, SlotTypeEnum slotType) {
        //obj 有两种情况, 一个正常类的实例或者一个Class对象: 来源于staticFieldBase方法
        //如果要读取一个类的static字段, 则obj会传入类对应的Class对象
        //这也意味着Unsafe应该无法正常处理Class类本身的字段,因为Unsafe的get/put方法没有区分是否为static字段
        //所以需要根据obj是否为Class对象来判断, 是Class对象则取对应类的static字段 否则取对应的正常字段
        const auto obj = frame.getLocalRef(1);
        auto offset = frame.getLocalI8(2);

        if (obj->type == OopTypeEnum::InstanceOop) {
            const auto instanceObj = CAST_INSTANCE_OOP(obj);
            auto dataPtr = instanceObj->data.get();
            const auto objClass = instanceObj->getInstanceClass();
            if (offset >= objClass->instanceSlotCount) {
                //static
                dataPtr = objClass->staticData.get();
                offset -= objClass->instanceSlotCount;
            }
            dataPtr += offset;
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        } else if (obj->type == OopTypeEnum::ObjArrayOop) {
            const auto arrayObj = static_cast<ObjArrayOop *>(obj);
            auto dataPtr = arrayObj->data.get();
            dataPtr += offset;
            actionSwitch(frame, actionType, dataPtr, slotType);
            return;
        } else {
            const auto typeArrayClass = static_cast<TypeArrayClass *>(obj->klass);
            const auto basicType = getBasicTypeByTypeArrayClassName(typeArrayClass->name);
            switch (basicType) {
                case BasicType::T_BOOLEAN:
                case BasicType::T_BYTE: {
                    const auto arrayObj = static_cast<ByteTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_CHAR: {
                    const auto arrayObj = static_cast<CharTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_SHORT: {
                    const auto arrayObj = static_cast<ShortTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_INT: {
                    const auto arrayObj = static_cast<IntTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_FLOAT: {
                    const auto arrayObj = static_cast<FloatTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_LONG: {
                    const auto arrayObj = static_cast<LongTypeArrayOop *>(obj);
                    auto dataPtr = arrayObj->data.get();
                    dataPtr += offset;
                    actionSwitch(frame, actionType, dataPtr, slotType);
                    return;
                }
                case BasicType::T_DOUBLE: {
                    const auto arrayObj = static_cast<DoubleTypeArrayOop *>(obj);
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