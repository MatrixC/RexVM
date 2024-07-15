#ifndef NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP
#define NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP

#include <unordered_map>
#include <mutex>
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
    const cstring JAVA_LANG_INVOKE_METHOD_TYPE_NAME = "java/lang/invoke/MethodType";

    //gc car
    std::unordered_map<InstanceOop *, InstanceOop *> MEMBER_NAME_CACHE_MAP;
    std::mutex MEMBER_NAME_CACHE_MAP_MUTEX;

    void cacheMemberName(InstanceOop *memberNameOop) {
        const auto type = CAST_INSTANCE_OOP(memberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal);
        std::lock_guard<std::mutex> lock(MEMBER_NAME_CACHE_MAP_MUTEX);
        MEMBER_NAME_CACHE_MAP[type] = memberNameOop;
    }

    InstanceOop *findMemberNameByMethodType(InstanceOop *methodTypeOop) {
        if (methodTypeOop == nullptr) {
            panic("memberType can't be nullptr");
        }
        std::lock_guard<std::mutex> lock(MEMBER_NAME_CACHE_MAP_MUTEX);
        const auto iter = MEMBER_NAME_CACHE_MAP.find(methodTypeOop);
        if (iter == MEMBER_NAME_CACHE_MAP.end()) {
            panic("can't find memberName");
        }
        return iter->second;
    }

    bool isStaticMethondHandleType(MethodHandleEnum kind) {
        return kind == MethodHandleEnum::REF_getStatic 
            || kind == MethodHandleEnum::REF_putStatic 
            || kind == MethodHandleEnum::REF_invokeStatic;
    }

    void setMemberNameClazzAndFlags(InstanceOop *memberNameOop, InstanceOop *mirrirOop, i4 newFlags) {
        memberNameOop->setFieldValue("flags", "I", Slot(newFlags));
        memberNameOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(mirrirOop)); 
    }

    cstring getDescriptor(Class *clazz, InstanceOop *type, const cstring &name) {
        cstring descriptor{};
        if (isMethodHandleInvoke(clazz->name, name)) {
            descriptor = METHOD_HANDLE_INVOKE_ORIGIN_DESCRITPR;
        } else {
            const auto typeClass = type->klass;
            const auto typeClassName = typeClass->name;
		    if (typeClassName == JAVA_LANG_INVOKE_METHOD_TYPE_NAME) {
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
                panic("error typeClassName");
	    	}
	    }
        return descriptor;
    }

    void methodHandlerResolve(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        if (memberNameOop == nullptr) {
            panic("can't be nullptr error throws");
            return;
        }
        auto &classLoader = *frame.getCurrentClassLoader();

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
        const auto isStatic = isStaticMethondHandleType(kind);
        if ((flags & MN_IS_METHOD) || (flags & MN_IS_CONSTRUCTOR)) {
            const auto resolveMethod = instanceClass->getMethod(name, descriptor, isStatic);
            const auto newFlags = flags | CAST_I4(resolveMethod->accessFlags);
            setMemberNameClazzAndFlags(memberNameOop, resolveMethod->klass.getMirrorOop(), newFlags);
        } else if (flags & MN_IS_FIELD) {
            const auto resolveField = instanceClass->getField(name, descriptor, isStatic);
            const auto newFlags = flags | CAST_I4(resolveField->accessFlags);
            setMemberNameClazzAndFlags(memberNameOop, resolveField->klass.getMirrorOop(), newFlags);
        } else if (flags & MN_IS_TYPE) {
            panic("error");
        }
        cacheMemberName(memberNameOop);
        frame.returnRef(memberNameOop);
    }

    void methodHandlerInit(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
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
        cacheMemberName(memberNameOop);
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

    void methodHandleInvoke(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto self = frame.getThisInstance();
        const auto methodType = CAST_INSTANCE_OOP(self->getFieldValue("type", "Ljava/lang/invoke/MethodType;").refVal);

        InstanceOop *mn = findMemberNameByMethodType(methodType);
        

        if (mn == nullptr) {
            panic("Error");
        }


        const auto klass = GET_MIRROR_INSTANCE_CLASS(mn->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto name = StringPool::getJavaString(CAST_INSTANCE_OOP(mn->getFieldValue("name", "Ljava/lang/String;").refVal));
        //const auto type = GET_MIRROR_INSTANCE_CLASS(mn->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto type = CAST_INSTANCE_OOP(mn->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto flags = mn->getFieldValue("flags", "I").i4Val;
        const auto kind = static_cast<MethodHandleEnum>((flags >> MN_REFERENCE_KIND_SHIFT) & MN_REFERENCE_KIND_MASK);
        const auto isStatic = kind == MethodHandleEnum::REF_invokeStatic;
        const auto descriptor = getDescriptor(klass, type, name);
        const auto methodPtr = klass->getMethod(name, descriptor, isStatic);

        if (kind == MethodHandleEnum::REF_invokeInterface 
            || kind == MethodHandleEnum::REF_invokeSpecial
            || kind == MethodHandleEnum::REF_invokeVirtual) {
        }
        

        std::vector<Slot> params;
        for (size_t i = 0; i < methodPtr->paramSlotSize; ++i) {
            params.emplace_back(frame.getLocal(i + 1));
        }

        const auto [slotType, result] = frame.runMethodManual(*methodPtr, params);

        frame.returnRef(nullptr);
        

    }

    void methodHandlerGetConstant(Frame &frame) {
        frame.returnI4(0);
    }



}

#endif