#ifndef NATIVE_CORE_JAVA_LANG_OBJECT_HPP
#define NATIVE_CORE_JAVA_LANG_OBJECT_HPP
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../mirror_oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../exception_helper.hpp"
#include <thread>
#include <bit>


namespace RexVM::Native::Core {

    void getClass(Frame &frame) {
        frame.returnRef(frame.getThis()->getClass()->getMirror(&frame));
    }

    void hashCode(Frame &frame) {
        frame.returnI4(CAST_I4(std::bit_cast<u8>(frame.getThis())));
    }

    template<typename T>
    T *getTypeArrayClone(Frame &frame, T *src) {
        const auto klass = CAST_TYPE_ARRAY_CLASS(src->getClass());
        const auto newOop = frame.mem.newTypeArrayOop(klass->elementType, src->getDataLength());
        auto newArray = static_cast<T *>(newOop);
        std::copy(src->data.get(), src->data.get() + src->getDataLength(), newArray->data.get());

        FOR_FROM_ZERO(src->getDataLength()) {
            newArray->data[i] = src->data[i];
        }
        return newArray;
    }

    ObjArrayOop *getObjArrayClone(Frame &frame, ObjArrayOop *src) {
        const auto klass = CAST_OBJ_ARRAY_CLASS(src->getClass());
        const auto newOop = frame.mem.newObjArrayOop(klass, src->getDataLength());
        for (size_t i = 0; i < src->getDataLength(); ++i) {
            newOop->data[i] = src->data[i];
        }
        return newOop;
    }

    //protected native Object clone() throws CloneNotSupportedException;
    void clone(Frame &frame) {
        const auto self = frame.getThis();
        const auto klass = self->getClass();
        switch (klass->getType()) {
            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto newInstance = frame.mem.newInstance((CAST_INSTANCE_OOP(self))->getInstanceClass());
                frame.returnRef((CAST_INSTANCE_OOP(self))->clone(newInstance));
                return;
            }

            case ClassTypeEnum::OBJ_ARRAY_CLASS:
                frame.returnRef(getObjArrayClone(frame, CAST_OBJ_ARRAY_OOP(self)));
                return;

            case ClassTypeEnum::TYPE_ARRAY_CLASS: {
                const auto typeArrayKlass = CAST_TYPE_ARRAY_CLASS(klass);
                switch (typeArrayKlass->elementType) {
                    case BasicType::T_BOOLEAN:
                    case BasicType::T_BYTE:
                        frame.returnRef(getTypeArrayClone(frame, CAST_BYTE_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_CHAR:
                        frame.returnRef(getTypeArrayClone(frame, CAST_CHAR_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_FLOAT:
                        frame.returnRef(getTypeArrayClone(frame, CAST_FLOAT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_DOUBLE:
                        frame.returnRef(getTypeArrayClone(frame, CAST_DOUBLE_TYPE_ARRAY_OOP(self)));
                        return;
                    
                    case BasicType::T_SHORT:
                        frame.returnRef(getTypeArrayClone(frame, CAST_SHORT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_INT:
                        frame.returnRef(getTypeArrayClone(frame, CAST_INT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_LONG:
                        frame.returnRef(getTypeArrayClone(frame, CAST_LONG_TYPE_ARRAY_OOP(self)));
                        return;
                    
                    default:
                        panic("error basic type");
                }
                break;
            }

            default:
                panic("error clone type");
        }
    }

    //public final native void notify();
    void notify(Frame &frame) {
        const auto self = frame.getThis();
        ASSERT_IF_NULL_THROW_NPE(self);
        self->notify_one();
    }

    //public final native void notifyAll();
    void notifyAll(Frame &frame) {
        const auto self = frame.getThis();
        ASSERT_IF_NULL_THROW_NPE(self);
        self->notify_all();
    }

    //public final native void wait(long timeout) throws InterruptedException;
    void wait(Frame &frame) {
        auto &currentThread = frame.thread;
        const auto self = frame.getThis(); //called Thread
        ASSERT_IF_NULL_THROW_NPE(self);
        const auto timeout = frame.getLocalI8(1);
        self->wait(currentThread, CAST_SIZE_T(timeout));
    }

}

#endif