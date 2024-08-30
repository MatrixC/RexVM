#ifndef NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP
#define NATIVE_CORE_JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_HPP

#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../method_handle.hpp"
#include "../../utils/class_utils.hpp"
#include "../../exception_helper.hpp"

namespace RexVM::Native::Core {

    void setMemberNameClazzAndFlags(InstanceOop *memberNameOop, InstanceOop *mirrirOop, i4 newFlags) {
        memberNameOop->setFieldValue("flags", "I", Slot(newFlags));
        memberNameOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(mirrirOop)); 
    }

    void methodHandlerInit(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto refOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto refClass = refOop->getClass();
        const auto methodMirrorClass = 
            frame.vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_METHOD);

        const auto fieldMirrorClass = 
            frame.vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_FIELD);

        const auto constructorMirrorClass = 
            frame.vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_CONSTRUCTOR);

        if (refClass == methodMirrorClass) {
            const auto slotId = refOop->getFieldValue("slot", "I").i4Val;
            const auto klass = GET_MIRROR_INSTANCE_CLASS(refOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
            const auto methodPtr = klass->methods[slotId].get();
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
            setMemberNameClazzAndFlags(memberNameOop, methodPtr->klass.getMirror(&frame), newFlags);
        } else if (refClass == fieldMirrorClass) {
            const auto slotId = refOop->getFieldValue("slot", "I").i4Val;
            const auto klass = GET_MIRROR_INSTANCE_CLASS(refOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
            const auto fieldPtr = klass->fields[slotId].get();
            auto newFlags = CAST_I4(fieldPtr->accessFlags) | MN_IS_FIELD;
            (void)newFlags;

            panic("error");
        } else if (refClass == constructorMirrorClass) {
            panic("error");
        } else {
            panic("error");
        }
        //cacheMemberName(memberNameOop);
    }

    void methodHandlerResolve(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        if (memberNameOop == nullptr) {
            panic("can't be nullptr error throws");
            return;
        }

        auto clazz = GET_MIRROR_CLASS(memberNameOop->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto name = VMStringHelper::getJavaString(CAST_INSTANCE_OOP(memberNameOop->getFieldValue("name", "Ljava/lang/String;").refVal));
        const auto type = CAST_INSTANCE_OOP(memberNameOop->getFieldValue("type", "Ljava/lang/Object;").refVal);
        const auto flags = memberNameOop->getFieldValue("flags", "I").i4Val;
        const auto kind = static_cast<MethodHandleEnum>((flags >> MN_REFERENCE_KIND_SHIFT) & MN_REFERENCE_KIND_MASK);

        if (clazz == nullptr) {
            panic("resolve error");
        } else if (clazz->getType() == ClassTypeEnum::TYPE_ARRAY_CLASS || clazz->getType() == ClassTypeEnum::OBJ_ARRAY_CLASS) {
            clazz = frame.mem.getClass(JAVA_LANG_OBJECT_NAME);
        }
        const auto instanceClass = CAST_INSTANCE_CLASS(clazz);
        const auto descriptor = methodHandleGetDescriptor(clazz, type, name);
        const auto isStatic = isStaticMethodHandleType(kind);
        if ((flags & MN_IS_METHOD) || (flags & MN_IS_CONSTRUCTOR)) {
            if (kind == MethodHandleEnum::REF_invokeInterface && !instanceClass->isInterface()) {
                throwAssignException(frame, "java/lang/IncompatibleClassChangeError", "Found class " + instanceClass->name + " but interface was expected");
                return;
            }

            const auto resolveMethod = instanceClass->getMethod(name, descriptor, isStatic);
            if (resolveMethod == nullptr) {
                //throwReflectiveOperationException(frame, instanceClass->name, name, descriptor);
                const auto message = cformat("{}.{}{}", getJavaClassName(instanceClass->name), name, descriptor);
                throwAssignException(frame, "java/lang/NoSuchMethodError", message);
                return;
            }
            const auto newFlags = flags | CAST_I4(resolveMethod->accessFlags);
            setMemberNameClazzAndFlags(memberNameOop, resolveMethod->klass.getMirror(&frame), newFlags);
        } else if (flags & MN_IS_FIELD) {
            const auto resolveField = instanceClass->getField(name, descriptor, isStatic);
            const auto newFlags = flags | CAST_I4(resolveField->accessFlags);
            setMemberNameClazzAndFlags(memberNameOop, resolveField->klass.getMirror(&frame), newFlags);
        } else if (flags & MN_IS_TYPE) {
            panic("error");
        }
        frame.returnRef(memberNameOop);
    }


    std::tuple<InstanceOop *, bool> methodHandleGetMemberName(Frame &frame, InstanceOop *self, std::vector<Slot> &prefixParam) {
        const auto className = self->getClass()->name;
        const auto methodParamSlotSize = frame.methodParamSlotSize;
        InstanceOop *memberNameOop = nullptr;
        if (startWith(className, "java/lang/invoke/DirectMethodHandle")) {
            //DirectMethodHandle及其多个子类
            memberNameOop = CAST_INSTANCE_OOP(self->getFieldValue("member", "Ljava/lang/invoke/MemberName;").refVal);
        } else if (className == "java/lang/invoke/BoundMethodHandle$Species_LL") {
            //argL0.klass = java/lang/invoke/DirectMethodHandle
            const auto argL0 = CAST_INSTANCE_OOP(self->getFieldValue("argL0", "Ljava/lang/Object;").refVal);
            const auto argL1 = CAST_INSTANCE_OOP(self->getFieldValue("argL1", "Ljava/lang/Object;").refVal);
            prefixParam.emplace_back(argL1);
            memberNameOop = CAST_INSTANCE_OOP(argL0->getFieldValue("member", "Ljava/lang/invoke/MemberName;").refVal);
        } else if (className == "java/lang/invoke/BoundMethodHandle$Species_L") {
            const auto argL0 = CAST_INSTANCE_OOP(self->getFieldValue("argL0", "Ljava/lang/Object;").refVal);
            if (methodParamSlotSize == 1 || methodParamSlotSize == 2) {
                //只有self和Species_L两个参数
                frame.returnRef(argL0);
                return std::make_tuple(nullptr, true);
            }

            const auto speciesL = CAST_INSTANCE_OOP(frame.getLocalRef(1));
            if (speciesL == nullptr) {
                frame.returnRef(argL0);
                return std::make_tuple(nullptr, true);
            }
            const auto directMethodHandle = CAST_INSTANCE_OOP(speciesL->getFieldValue("argL0", "Ljava/lang/Object;").refVal);
            memberNameOop = CAST_INSTANCE_OOP(directMethodHandle->getFieldValue("member", "Ljava/lang/invoke/MemberName;").refVal);

            const auto passParams = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(2));
            for (size_t i = 0; i < passParams->getDataLength(); ++i) {
                prefixParam.emplace_back(passParams->data[i]);
            }
        } else if (className == "java/lang/invoke/MethodHandleImpl$IntrinsicMethodHandle"
                || className == "java/lang/invoke/MethodHandleImpl$WrappedMember"
                || className == "java/lang/invoke/MethodHandleImpl$AsVarargsCollector") {
            const auto directMethodHandle = CAST_INSTANCE_OOP(self->getFieldValue("target", "Ljava/lang/invoke/MethodHandle;").refVal);
            memberNameOop = CAST_INSTANCE_OOP(directMethodHandle->getFieldValue("member", "Ljava/lang/invoke/MemberName;").refVal);
        } else {
            panic("error");
        }
        return std::make_tuple(memberNameOop, false);
    }

    void methodHandleInvokeBoxOrUnBox(
        Frame &frame,
        Slot val, 
        SlotTypeEnum type,
        Method *methodPtr,
        size_t index,
        std::vector<Slot> &params
    ) {
        //const auto paramSlotType = methodPtr->paramSlotType[index];

        const auto &methodParamType = methodPtr->paramType;
        if (index >= methodParamType.size()) {
            //like java.lang.invoke.LambdaMetafactory#altMetafactory
            //fold last param
            index = methodParamType.size() - 1;
        }
        const auto paramType = methodParamType[index];
        const auto paramSlotType = getSlotTypeByPrimitiveClassName(paramType);
        if (type == paramSlotType) {
            params.emplace_back(val);
            if (isWideSlotType(type)) {
                //compressParamsWithType里只保留了有值的那个 所以补一个
                params.emplace_back(ZERO_SLOT);
            }
            return;
        }
        const auto paramClass = frame.mem.getClass(paramType);
        const auto className = paramClass->name;
        //两种情况 传参是REF 函数需要Primitive 或者相反
        if (type == SlotTypeEnum::REF) {
            //Unbox
            if (paramClass->type != ClassTypeEnum::PRIMITIVE_CLASS) {
                panic("error");
            }
            const auto primitiveClass = CAST_PRIMITIVE_CLASS(paramClass);
            const auto slotVal = primitiveClass->getValueFromBoxingOop(CAST_INSTANCE_OOP(val.refVal));
            params.emplace_back(slotVal);
            if (primitiveClass->isWideType()) {
                params.emplace_back(ZERO_SLOT);
            } 
        } else {
            //Box
            if (className == JAVA_LANG_INTEGER_NAME) {
                params.emplace_back(frame.mem.newIntegerOop(val.i4Val));
            } else if (className == JAVA_LANG_LONG_NAME) {
                params.emplace_back(frame.mem.newLongOop(val.i8Val));
            } else if (className == JAVA_LANG_FLOAT_NAME) {
                params.emplace_back(frame.mem.newFloatOop(val.f4Val));
            } else if (className == JAVA_LANG_DOUBLE_NAME) {
                params.emplace_back(frame.mem.newDoubleOop(val.f8Val));
            } else if (className == JAVA_LANG_BOOLEAN_NAME) {
                params.emplace_back(frame.mem.newBooleanOop(val.i4Val));
            } else if (className == JAVA_LANG_BYTE_NAME) {
                params.emplace_back(frame.mem.newByteOop(val.i4Val));
            } else if (className == JAVA_LANG_CHARACTER_NAME) {
                params.emplace_back(frame.mem.newCharOop(val.i4Val));
            } else if (className == JAVA_LANG_SHORT_NAME) {
                params.emplace_back(frame.mem.newShortOop(val.i4Val));
            } else {
                panic("error");
            }
        }
    }

    //处理像java.lang.invoke.LambdaMetafactory#altMetafactory这样最后一个参数是Object[]的情况
    //在它被调用是通过 bootstrapMethod.invoke(caller, name, type, argv[0], argv[1], argv[2], argv[3], argv[4]);
    //这种方式调用的 所以要自动把最后几个参数折叠到Object[]...
    void methodHandleProcessMethodObjectArrayParam(Frame &frame, Method *methodPtr, std::vector<Slot> &params) {
        if (methodPtr->paramSlotSize == params.size()) {
            return;
        } else if (methodPtr->paramSlotSize > params.size()) {
            panic("params length error");
        }

        const auto lastParamType = methodPtr->paramType.back();

        if (lastParamType == "[Ljava/lang/Object;") {
            //最后一个参数是数组
            const auto foldParamSize = params.size() - methodPtr->paramSlotSize + 1;
            const auto arrayOop = frame.mem.newObjectObjArrayOop(foldParamSize);
            for (size_t i = 0; i < foldParamSize; ++i) {
                arrayOop->data[i] = params[methodPtr->paramSlotSize - 1 + i].refVal;
            }
            //删除要被折叠的元素
            params.erase(params.end() - foldParamSize, params.end());
            params.emplace_back(arrayOop);
        } else {
            panic("params type error");
        }
    }

    std::vector<std::tuple<Slot, SlotTypeEnum>> methodHandleBuildInvokeMethodParams(
        Frame &frame, 
        Method *methodPtr, 
        const std::vector<Slot>& refPrefixParam,
        bool isConstructor
    ) {
        std::vector<std::tuple<Slot, SlotTypeEnum>> params;

        //auto paramSlotSize = methodPtr->paramSlotSize;
        //-1是为了忽略掉MethodHandleOop
        auto paramSlotSize = frame.methodParamSlotSize - 1;
        if (isConstructor) {
            paramSlotSize += 1;
        }

        //这行代码是因为在bindTo情况下 frame.methodParamSlotSize有问题 少了this
        paramSlotSize = std::max(paramSlotSize, methodPtr->paramSlotSize);

        params.reserve(paramSlotSize + refPrefixParam.size());

        for (const auto &item : refPrefixParam) {
            params.emplace_back(item, SlotTypeEnum::REF);
        }
        paramSlotSize -= refPrefixParam.size();

        for (size_t i = 0; i < paramSlotSize; ++i) {
            //1是self
            params.emplace_back(frame.getLocalWithType(i + 1));
        }
        return params;
    }

    void methodHandleInvoke(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)

        std::vector<Slot> prefixParam;
        const auto [memberNameOop, exitInvoke] = methodHandleGetMemberName(frame, self, prefixParam);
        if (exitInvoke) {
            return;
        }

        if (memberNameOop == nullptr) {
            panic("error: memberName is null");
        }

        auto [klass, name, type, flags, kind, isStatic, descriptor]
            = methodHandleGetFieldFromMemberName(memberNameOop);

        //const auto memberNameMirrorOop = CAST_MIRROR_OOP(memberNameOop);
        //const auto methodPtr = memberNameMirrorOop->getMemberNameMethod();
        const auto methodPtr = klass->getMethod(name, descriptor, isStatic);

        if (methodPtr == nullptr) {
            panic("error: methodPtr is null");
        }

        if ((kind == MethodHandleEnum::REF_invokeSpecial || kind == MethodHandleEnum::REF_newInvokeSpecial) 
                && methodPtr->isConstructor()) {
            //Constructor
            const auto newInstance = frame.mem.newInstance( &methodPtr->klass);
            const auto paramsWithType = methodHandleBuildInvokeMethodParams(frame, methodPtr, { Slot(newInstance) }, true);
            frame.runMethodManualTypes(*methodPtr, paramsWithType);
            frame.returnRef(newInstance);
            return;
        }

        auto paramsWithType = methodHandleBuildInvokeMethodParams(frame, methodPtr, prefixParam, false);
        //box or unbox 处理
        //这里比反射的invoke处理复杂 反射传参只会是Object数组 所以Slot和paramType的index是直接对应的
        //但MH的传参有可能是PrimitiveType 也可能是Integer等Box类型
        //会发生一种比较复杂的情况 假设函数是 void say(Long i); 但是传参是一个 long类型 需要box且paramSlotSize对不上 
        std::vector<std::tuple<Slot, SlotTypeEnum>> compressParamsWithType;
        for (size_t i = 0; i < paramsWithType.size(); ++i) {
            const auto [val, slotType] = paramsWithType[i];
            compressParamsWithType.emplace_back(paramsWithType[i]);
            if (isWideSlotType(slotType)) {
                //跳过 只保留第一个带值的Slot 这样最终的Slot数可以保证和函数的参数数量一致
                i += 1;
            }
        }

        std::vector<Slot> params;
        params.reserve(compressParamsWithType.size());
        const auto methodIsStatic = methodPtr->isStatic();
        const auto startIndex = methodIsStatic ? 0 : 1;
        if (!methodIsStatic) {
            params.emplace_back(std::get<0>(compressParamsWithType[0]));
        }

        for (size_t i = startIndex; i < compressParamsWithType.size(); ++i) {
            const auto [val, slotType] = compressParamsWithType[i];
            const auto methodIndex = methodIsStatic ? i : i - 1;
            methodHandleInvokeBoxOrUnBox(frame, val, slotType, methodPtr, methodIndex, params);
        }

        methodHandleProcessMethodObjectArrayParam(frame, methodPtr, params);

        const auto [result, slotType] = frame.runMethodManual(*methodPtr, params);
        if (frame.markThrow) {
            return;
        }
        const auto returnClass = frame.mem.getClass(methodPtr->returnType);
        ref oopResult = nullptr;
        if (slotType != SlotTypeEnum::NONE) {
            if (returnClass->type == ClassTypeEnum::PRIMITIVE_CLASS) {
                const auto returnPrimitiveClass = CAST_PRIMITIVE_CLASS(returnClass);
                oopResult = returnPrimitiveClass->getBoxingOopFromValue(result, frame);
            } else {
                oopResult = result.refVal;
            }
        }
        frame.returnRef(oopResult);

    }

    void methodHandlerGetConstant(Frame &frame) {
        frame.returnI4(0);
    }

    void methodHandleGetMembers(Frame &frame) {
        frame.returnI4(50);
    }

    void methodHandleObjectFieldOffset(Frame &frame) {
        const auto memberNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        ASSERT_IF_NULL_THROW_NPE(memberNameOop)
        auto [klass, name, type, flags, kind, isStatic, descriptor] = methodHandleGetFieldFromMemberName(memberNameOop);
        const auto typeClass = GET_MIRROR_INSTANCE_CLASS(type);
        const auto field = klass->getField(name, getDescriptorByClass(typeClass), isStatic);
        frame.returnI8(field->slotId);
    }

}

#endif