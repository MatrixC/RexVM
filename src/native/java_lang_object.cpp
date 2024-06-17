#include "java_lang_object.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../vm.hpp"
#include "../memory.hpp"
#include <bit>

namespace RexVM::Native {

    void getClass(Frame &frame) {
        frame.returnRef(frame.getThis()->klass->getMirrorOop());
    }

    void hashCode(Frame &frame) {
        frame.returnI4(static_cast<i4>(std::bit_cast<u8>(frame.getThis())));
    }

    template<typename T>
    T *getTypeArrayClone(OopManager *oopManager, T *src) {
        const auto klass = static_cast<TypeArrayClass *>(src->klass);
        const auto newOop = oopManager->newTypeArrayOop(klass->elementType, src->dataLength);
        auto newArray = static_cast<T *>(newOop);
        for (auto i = 0; i < src->dataLength; ++i) {
            newArray->data[i] = src->data[i];
        }
        return newArray;
    }

    ObjArrayOop *getObjArrayClone(OopManager *oopManager, ObjArrayOop *src) {
        const auto klass = static_cast<ObjArrayClass *>(src->klass);
        const auto newOop = oopManager->newObjArrayOop(klass, src->dataLength);
        for (auto i = 0; i < src->dataLength; ++i) {
            newOop->data[i] = src->data[i];
        }
        return newOop;
    }

    void clone(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto self = frame.getThis();
        const auto klass = self->klass;
        switch (klass->type) {
            case ClassTypeEnum::InstanceClass:
                frame.returnRef((dynamic_cast<InstanceOop *>(self))->clone(*oopManager));
                return;

            case ClassTypeEnum::ObjArrayClass:
                frame.returnRef(getObjArrayClone(oopManager.get(), dynamic_cast<ObjArrayOop *>(self)));
                return;

            case ClassTypeEnum::TypeArrayClass: {
                const auto typeArrayKlass = dynamic_cast<const TypeArrayClass *>(klass);
                switch (typeArrayKlass->elementType) {
                    case BasicType::T_BOOLEAN:
                    case BasicType::T_BYTE:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<ByteTypeArrayOop *>(self)));
                        return;

                    case BasicType::T_CHAR:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<CharTypeArrayOop *>(self)));
                        return;

                    case BasicType::T_FLOAT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<FloatTypeArrayOop *>(self)));
                        return;

                    case BasicType::T_DOUBLE:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<DoubleTypeArrayOop *>(self)));
                        return;
                    
                    case BasicType::T_SHORT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<ShortTypeArrayOop *>(self)));
                        return;

                    case BasicType::T_INT:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<IntTypeArrayOop *>(self)));
                        return;

                    case BasicType::T_LONG:
                        frame.returnRef(getTypeArrayClone(oopManager.get(), dynamic_cast<LongTypeArrayOop *>(self)));
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

    void notifyAll(Frame &frame) {
    }



}