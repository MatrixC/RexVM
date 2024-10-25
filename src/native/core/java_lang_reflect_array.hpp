#ifndef NATIVE_CORE_JAVA_LANG_REFLECT_ARRAY_HPP
#define NATIVE_CORE_JAVA_LANG_REFLECT_ARRAY_HPP
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../memory.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    //static native Object newArray(Class<?> componentType, int length)
    void newArray(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(0));
        if (mirrorOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto length = frame.getLocalI4(1);
        const auto mirrorClass = mirrorOop->getMirrorClass();
        switch (mirrorClass->type) {
            case ClassTypeEnum::PRIMITIVE_CLASS: {
                const auto primitiveClass = CAST_PRIMITIVE_CLASS(mirrorClass);
                const auto arrayObj = frame.mem.newTypeArrayOop(primitiveClass->getBasicType(), length);
                frame.returnRef(arrayObj);
                return;
            }

            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto arrayClass = frame.mem.getObjectArrayClass(*mirrorClass);
                const auto arrayObj = frame.mem.newObjArrayOop(arrayClass, length);
                frame.returnRef(arrayObj);
                return;
            }

            default:
                //Array Class
                const auto arrayClass = frame.mem.getObjectArrayClass(*mirrorClass);
                const auto arrayObj = frame.mem.newObjArrayOop(arrayClass, length);
                frame.returnRef(arrayObj);
                return;
        }
    }

}


#endif