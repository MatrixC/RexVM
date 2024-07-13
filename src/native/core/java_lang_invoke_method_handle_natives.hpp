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
#include "../../utils/class_utils.hpp"


namespace RexVM::Native::Core {

    void getConstant(Frame &frame) {
        frame.returnI4(0);
    }

    cstring getDescriptor(Class *clazz, InstanceOop *type, const cstring &name) {
        cstring descriptor{};
        if (clazz->name == "java/lang/invoke/MethodHandle" &&
				(name == "invoke"
				|| name == "invokeBasic"
				|| name == "invokeExact"
				|| name == "invokeWithArauments"
				|| name == "linkToSpecial"
				|| name == "linkToStatic"
				|| name == "linkToVirtual"
				|| name == "linkToInterface"))  {
		    descriptor = "([Ljava/lang/Object;)Ljava/lang/Object;";
	    } else {
            const auto typeClass = type->klass;
            const auto typeClassName = typeClass->name;
		    if (typeClassName == "java/lang/invoke/MethodType") {
			    descriptor += "(";
                const auto ptypes = CAST_OBJ_ARRAY_OOP(type->getFieldValue("ptypes", "[Ljava/lang/Class;").refVal);
                for (size_t i = 0; i < ptypes->dataLength; ++i) {
                    const auto classMirrorOop = CAST_MIRROR_OOP(ptypes->data[i]);
                    const auto mirrorClass = classMirrorOop->mirrorClass;
                    descriptor += getDescriptorByClass(mirrorClass);
                }
			    descriptor += ")";
                const auto rtype = CAST_MIRROR_OOP(type->getFieldValue("rtype", "Ljava/lang/Class;").refVal);
			    descriptor += getDescriptorByClass(rtype->mirrorClass);
		    } else if (typeClassName == JAVA_LANG_CLASS_NAME) {
                const auto mirrorClass = CAST_MIRROR_OOP(type)->mirrorClass;
                descriptor = getDescriptorByClass(mirrorClass);
		    } else {
                panic("error");
	    	}
	    }
        return descriptor;
    }

    void resolve(Frame &frame) {
        const auto memeberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
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

        auto clazz = (CAST_MIRROR_OOP(memeberNameOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal))->mirrorClass;
        const auto name = StringPool::getJavaString(CAST_INSTANCE_OOP(memeberNameOop->getFieldValue("name", "Ljava/lang/String;").refVal));
        //const auto type = CAST_INSTANCE_OOP(memeberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto flags = memeberNameOop->getFieldValue("type", "I").i4Val;
        //const auto kind = static_cast<MethodHandleEnum>((flags & 0xF000000) >> 24);

        if (clazz == nullptr) {
            panic("resolve error");
        } else if (clazz->type == ClassTypeEnum::TypeArrayClass || clazz->type == ClassTypeEnum::ObjArrayClass) {
            clazz = frame.vm.bootstrapClassLoader->getClass(JAVA_LANG_OBJECT_NAME);
        }

        //const auto mirrorInstanceClass = CAST_INSTANCE_CLASS(clazz);
        //const auto descriptor = getDescriptor(clazz, type, name);
        

        if (flags & 0x10000) {
            //const auto method = mirrorInstanceClass->getMethod(name, descriptor, false);
            //auto result = frame.vm.oopManager->newInstance(mirrorInstanceClass);
            



        } else if (flags & 0x20000) {

        } else if (flags & 0x40000) {

        } else {
            panic("resolve error");
        }
        






    }

}

#endif