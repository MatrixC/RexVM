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

    // MemberName
    // The JVM uses values of -2 and above for vtable indexes.
    // Field values are simple positive offsets.
    // Ref: src/share/vm/oops/methodOop.hpp
    // This value is negative enough to avoid such numbers,
    // but not too negative.
    constexpr i4 MN_IS_METHOD           = 0x00010000; // method (not constructor)
    constexpr i4 MN_IS_CONSTRUCTOR      = 0x00020000; // constructor
    constexpr i4 MN_IS_FIELD            = 0x00040000; // field
    constexpr i4 MN_IS_TYPE             = 0x00080000; // nested type
    constexpr i4 MN_CALLER_SENSITIVE    = 0x00100000; // @CallerSensitive annotation detected
    constexpr i4 MN_REFERENCE_KIND_SHIFT = 24; // refKind
    constexpr i4 MN_REFERENCE_KIND_MASK = 0x0F000000 >> MN_REFERENCE_KIND_SHIFT;
    constexpr i4 MN_SEARCH_SUPERCLASSES = 0x00100000;
    constexpr i4 MN_SEARCH_INTERFACES   = 0x00200000;

    void methodHandlerGetConstant(Frame &frame) {
        frame.returnI4(0);
    }

    cstring getDescriptor(Class *clazz, InstanceOop *type, const cstring &name) {
        cstring descriptor{};
        if (clazz->name == "java/lang/invoke/MethodHandle" &&
				(name == "invoke"
				|| name == "invokeBasic"
				|| name == "invokeExact"
				|| name == "invokeWithArguments"
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

    void methodHandlerResolve(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        if (memberNameOop == nullptr) {
            panic("can't be error throws");
            return;
        }
        auto &classLoader = *frame.getCurrentClassLoader();

//        private Class<?> clazz;       // class in which the method is defined
//        private String   name;        // may be null if not yet materialized
//        private Object   type;        // may be null if not yet materialized
//        private int      flags;

        auto clazz = GET_MIRROR_CLASS(memberNameOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto name = StringPool::getJavaString(CAST_INSTANCE_OOP(memberNameOop->getFieldValue("name", "Ljava/lang/String;").refVal));
        const auto type = CAST_INSTANCE_OOP(memberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto flags = memberNameOop->getFieldValue("flags", "I").i4Val;
        const auto kind = static_cast<MethodHandleEnum>((flags >> MN_REFERENCE_KIND_SHIFT) & MN_REFERENCE_KIND_MASK);

        if (clazz == nullptr) {
            panic("resolve error");
        } else if (clazz->type == ClassTypeEnum::TypeArrayClass || clazz->type == ClassTypeEnum::ObjArrayClass) {
            clazz = classLoader.getClass(JAVA_LANG_OBJECT_NAME);
        }
        const auto instanceClass = CAST_INSTANCE_CLASS(clazz);
        const auto descriptor = getDescriptor(clazz, type, name);
        if (flags & MN_IS_METHOD) {
            const auto isStatic = kind == MethodHandleEnum::REF_invokeStatic;
            const auto resolveMethod = instanceClass->getMethod(name, descriptor, isStatic);
            const auto newFlags = flags | CAST_I4(resolveMethod->accessFlags);
            memberNameOop->setFieldValue("flags", "I", Slot(newFlags));
            memberNameOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(resolveMethod->klass.getMirrorOop()));
            frame.returnRef(memberNameOop);
        } else if (flags & MN_IS_CONSTRUCTOR) {
            panic("error");
        } else if (flags & MN_IS_FIELD) {
            const auto isStatic = kind == MethodHandleEnum::REF_getStatic || kind == MethodHandleEnum::REF_putStatic;
            const auto resolveField = instanceClass->getField(name, descriptor, isStatic);
            const auto newFlags = flags | CAST_I4(resolveField->accessFlags);
            memberNameOop->setFieldValue("flags", "I", Slot(newFlags));
            memberNameOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(resolveField->klass.getMirrorOop()));
            frame.returnRef(memberNameOop);
        } else if (flags & MN_IS_TYPE) {
            panic("error");
        }
    }

    void methodHandlerInit(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto refOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto refClassName = refOop->klass->name;
        //const auto flags = memberNameOop->getFieldValue("flags", "I").i4Val;

        if (refClassName == "java/lang/reflect/Method") {
            const auto slotId = refOop->getFieldValue("slot", "I").i4Val;
            const auto klass = GET_MIRROR_INSTANCE_CLASS(refOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
            const auto methodPtr = klass->methods[slotId].get();
            //const auto newFlags = flags | CAST_I4(methodPtr->accessFlags);
            auto newFlags = CAST_I4(methodPtr->accessFlags) | MN_IS_METHOD;

            if (methodPtr->isPrivate() && !methodPtr->isStatic()) {
                newFlags |= (CAST_I4(MethodHandleEnum::REF_invokeSpecial) << MN_REFERENCE_KIND_SHIFT);
            } else if (methodPtr->isStatic()) {
                newFlags |= (CAST_I4(MethodHandleEnum::REF_invokeStatic) << MN_REFERENCE_KIND_SHIFT);
            } else {
                if (klass->isInterface()) {
                    newFlags |= (CAST_I4(MethodHandleEnum::REF_invokeInterface) << MN_REFERENCE_KIND_SHIFT);
                } else {
                    newFlags |= (CAST_I4(MethodHandleEnum::REF_invokeVirtual) << MN_REFERENCE_KIND_SHIFT);
                }
            }

            memberNameOop->setFieldValue("flags", "I", Slot(newFlags));
            memberNameOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(methodPtr->klass.getMirrorOop()));
        } else if (refClassName == "java/lang/reflect/Field") {
            panic("error");
        } else if (refClassName == "java/lang/reflect/Constructor") {
            panic("error");
        } else {
            panic("error");
        }
    }

    void methodHandleObjectFieldOffset(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto klass = GET_MIRROR_INSTANCE_CLASS(memberNameOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto name = StringPool::getJavaString(CAST_INSTANCE_OOP(memberNameOop->getFieldValue("name", "Ljava/lang/String;").refVal));
        const auto type = GET_MIRROR_INSTANCE_CLASS(memberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto flags = memberNameOop->getFieldValue("flags", "I").i4Val;
        const auto kind = static_cast<MethodHandleEnum>((flags >> MN_REFERENCE_KIND_SHIFT) & MN_REFERENCE_KIND_MASK);
        const auto isStatic = kind == MethodHandleEnum::REF_getStatic || kind == MethodHandleEnum::REF_putStatic;
        const auto field = klass->getField(name, getDescriptorByClass(type), isStatic);
        frame.returnI8(field->slotId);
    }

    void methodHandleGetMembers(Frame &frame) {
        frame.returnI4(1000);
    }



}

#endif