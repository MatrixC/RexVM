#ifndef NATIVE_CORE_JAVA_LANG_REFLECT_ARRAY_HPP
#define NATIVE_CORE_JAVA_LANG_REFLECT_ARRAY_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../memory.hpp"
#include "../../constant_pool.hpp"

namespace RexVM::Native::Core {

    //static native Object newArray(Class<?> componentType, int length)
    void newArray(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(0));
        if (mirrorOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto length = frame.getLocalI4(1);
        const auto &oopManager = frame.vm.oopManager;
        const auto mirrorClass = mirrorOop->mirrorClass;
        switch (mirrorClass->type) {
            case ClassTypeEnum::PRIMITIVE_CLASS: {
                const auto primitiveClass = CAST_PRIMITIVE_CLASS(mirrorClass);
                const auto arrayObj = oopManager->newTypeArrayOop(primitiveClass->getBasicType(), length);
                frame.returnRef(arrayObj);
                return;
            }

            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto arrayClass = frame.getCurrentClassLoader()->getObjectArrayClass(mirrorClass->name);
                const auto arrayObj = oopManager->newObjArrayOop(arrayClass, length);
                frame.returnRef(arrayObj);
                return;
            }

            default:
                const auto arrayClass = frame.getCurrentClassLoader()->getObjectArrayClass("[" + mirrorClass->name);
                const auto arrayObj = oopManager->newObjArrayOop(arrayClass, length);
                frame.returnRef(arrayObj);
                return;
        }
    }

}


#endif