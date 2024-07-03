#ifndef NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP
#define NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP

#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../invoke_dynamic.hpp"


namespace RexVM::Native::Core {

    void getConstant(Frame &frame) {
        frame.returnI4(0);
    }

    void resolve(Frame &frame) {
        const auto memeberNameOop = static_cast<InstanceOop *>(frame.getLocalRef(0));
        if (memeberNameOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }

        /*
        private Class<?> clazz;       // class in which the method is defined
        private String   name;        // may be null if not yet materialized
        private Object   type;        // may be null if not yet materialized
        private int      flags;  
        */

        const auto clazz = static_cast<MirrorOop *>(memeberNameOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        auto mirrorClass = clazz->mirrorClass;

        const auto name = memeberNameOop->getFieldValue("name", "Ljava/lang/String;").refVal;
        const auto type = memeberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal;
        const auto flags = memeberNameOop->getFieldValue("type", "I").i4Val;

        const auto kind = static_cast<MethodHandleEnum>((flags & 0xF000000) >> 24);
        if (mirrorClass->type == ClassTypeEnum::TypeArrayClass || mirrorClass->type == ClassTypeEnum::ObjArrayClass) {
            mirrorClass = frame.vm.bootstrapClassLoader->getClass(JAVA_LANG_OBJECT_NAME);
        }

        const auto mirrorInstanceClass = static_cast<InstanceClass *>(mirrorClass);
        






    }

}

#endif