#ifndef NATIVE_CORE_JAVA_LANG_OBJECT_HPP
#define NATIVE_CORE_JAVA_LANG_OBJECT_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include <thread>
#include <bit>


namespace RexVM::Native::Core {

    void getClass(Frame &frame) {
        frame.returnRef(frame.getThis()->klass->getMirrorOop());
    }

    void hashCode(Frame &frame) {
        frame.returnI4(CAST_I4(std::bit_cast<u8>(frame.getThis())));
    }

    template<typename T>
    T *getTypeArrayClone(OopManager *oopManager, T *src) {
        const auto klass = CAST_TYPE_ARRAY_CLASS(src->klass);
        const auto newOop = oopManager->newTypeArrayOop(klass->elementType, src->dataLength);
        auto newArray = static_cast<T *>(newOop);
        std::copy(src->data.get(), src->data.get() + src->dataLength, newArray->data.get());

        for (size_t i = 0; i < src->dataLength; ++i) {
            newArray->data[i] = src->data[i];
        }
        return newArray;
    }

    ObjArrayOop *getObjArrayClone(OopManager *oopManager, ObjArrayOop *src) {
        const auto klass = CAST_OBJ_ARRAY_CLASS(src->klass);
        const auto newOop = oopManager->newObjArrayOop(klass, src->dataLength);
        for (size_t i = 0; i < src->dataLength; ++i) {
            newOop->data[i] = src->data[i];
        }
        return newOop;
    }

    //protected native Object clone() throws CloneNotSupportedException;
    void clone(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto self = frame.getThis();
        const auto klass = self->klass;
        switch (klass->type) {
            case ClassTypeEnum::INSTANCE_CLASS:
                frame.returnRef((CAST_INSTANCE_OOP(self))->clone(*oopManager));
                return;

            case ClassTypeEnum::OBJ_ARRAY_CLASS:
                frame.returnRef(getObjArrayClone(oopManager.get(), CAST_OBJ_ARRAY_OOP(self)));
                return;

            case ClassTypeEnum::TYPE_ARRAY_CLASS: {
                const auto typeArrayKlass = CAST_TYPE_ARRAY_CLASS(klass);
                switch (typeArrayKlass->elementType) {
                    case BasicType::T_BOOLEAN:
                    case BasicType::T_BYTE:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_BYTE_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_CHAR:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_CHAR_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_FLOAT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_FLOAT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_DOUBLE:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_DOUBLE_TYPE_ARRAY_OOP(self)));
                        return;
                    
                    case BasicType::T_SHORT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_SHORT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_INT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_INT_TYPE_ARRAY_OOP(self)));
                        return;

                    case BasicType::T_LONG:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), CAST_LONG_TYPE_ARRAY_OOP(self)));
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
        self->monitorCv.notify_one();
    }

        //public final native void notifyAll();
    void notifyAll(Frame &frame) {
        const auto self = frame.getThis();
        self->monitorCv.notify_all();
    }

    //public final native void wait(long timeout) throws InterruptedException;
    void wait(Frame &frame) {
        auto &currentThread = frame.thread;
        const auto self = frame.getThis(); //called Thread
        const auto timeout = frame.getLocalI8(1);
        std::unique_lock<std::recursive_mutex> lock(self->monitorMtx, std::adopt_lock);
        const auto backupStatus = currentThread.getStatus();
        currentThread.setStatus(ThreadStatusEnum::WAITING);
        if (timeout == 0) [[likely]] {
            self->monitorCv.wait(lock);
        } else {
            self->monitorCv.wait_for(lock, std::chrono::microseconds(timeout));
        }
        currentThread.setStatus(backupStatus);
    }

}

#endif