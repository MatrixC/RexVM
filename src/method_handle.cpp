#include "method_handle.hpp"
#include "memory.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "string_pool.hpp"
#include "oop.hpp"
#include "mirror_oop.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "frame.hpp"
#include "thread.hpp"
#include "utils/descriptor_parser.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    bool isMethodHandleInvoke(const cview className, const cview memberName) {
        return (className == "java/lang/invoke/MethodHandle" && 
            (memberName == "invoke"
                || memberName == "invokeBasic"
                || memberName == "invokeExact"
                || memberName == "invokeWithArauments"
                || memberName == "linkToSpecial"
                || memberName == "linkToStatic"
                || memberName == "linkToVirtual"
                || memberName == "linkToInterface"));
    }

    bool isStaticMethodHandleType(MethodHandleEnum kind) {
        return kind == MethodHandleEnum::REF_getStatic
               || kind == MethodHandleEnum::REF_putStatic
               || kind == MethodHandleEnum::REF_invokeStatic;
    }

    //代替MethodType.fromMethodDescriptorString
    InstanceOop *createMethodType(Frame &frame, cview methodDescriptor) {
        const auto [paramType, returnTypeType] = parseMethodDescriptor(methodDescriptor);
        const auto returnTypeOop = frame.mem.getClass(returnTypeType)->getMirror(&frame);
        const auto classArrayOop = frame.mem.newClassObjArrayOop(paramType.size());

        size_t i = 0;
        for (const auto &className: paramType) {
            classArrayOop->data[i++] = frame.mem.getClass(className)->getMirror(&frame);
        }
        const auto runtimeMethodTypeClass = frame.mem.getInstanceClass("java/lang/invoke/MethodType");
        const auto makeImplMethod = runtimeMethodTypeClass->getMethod("makeImpl" "(Ljava/lang/Class;[Ljava/lang/Class;Z)Ljava/lang/invoke/MethodType;", true);

        //必须调用 makeImpl 自己创建会有点问题
        const auto [methodTypeSlot, _] = frame.runMethodManual(*makeImplMethod, { 
            Slot(returnTypeOop), 
            Slot(classArrayOop),
            Slot(CAST_I8(1)) //true
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(methodTypeSlot.refVal);
    }

    InstanceOop *createMethodType(Frame &frame, const ConstantMethodTypeInfo *methodTypeInfo) {
        const auto &constantPool = frame.klass.constantPool;
        const auto methodDescriptor = getConstantStringFromPool(constantPool, methodTypeInfo->descriptorIndex);
        return createMethodType(frame, methodDescriptor);
    }

    InstanceOop *createMethodHandle(
        Frame &frame, 
        ConstantMethodHandleInfo *methodHandleInfo, 
        InstanceClass *callerClass
    ) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;

        const auto methodHandleNativesClass = frame.mem.getInstanceClass("java/lang/invoke/MethodHandleNatives");
        const auto linkMethodHandleConstantMethod = methodHandleNativesClass->getMethod("linkMethodHandleConstant" "(Ljava/lang/Class;ILjava/lang/Class;Ljava/lang/String;Ljava/lang/Object;)Ljava/lang/invoke/MethodHandle;", true);

        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        const auto kind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        Slot type;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
            case MethodHandleEnum::REF_getStatic:
            case MethodHandleEnum::REF_putField:
            case MethodHandleEnum::REF_putStatic:
                type = Slot(frame.mem.getClass(getClassNameByFieldDescriptor(methodHandleMemberDescriptor))->getMirror(&frame));
                break;

            default:
                type = Slot(createMethodType(frame, methodHandleMemberDescriptor));
                break;
        }

        const auto methodHandleClass = frame.mem.getInstanceClass(methodHandleClassName);
        const auto methodHandleClassMirrorOop = methodHandleClass->getMirror(&frame);
        const auto methodHandleMemberNameOop = frame.mem.getInternString(methodHandleMemberName);

        //上面 methodHandleMemberNameOop 的分配必须靠近下面的 runMethodManual
        //在native方法中分配的内存 一定要直接传参 不要在在分配内存中又调用其他函数
        //否则被分配的对象极易容易被gc直接释放 导致后续函数拿到的是一个无效指针
        //这个问题查了我好久好久

        std::vector<Slot> linkMethodHandleParam;
        linkMethodHandleParam.reserve(5);
        linkMethodHandleParam.emplace_back(callerClass->getMirror(&frame));
        linkMethodHandleParam.emplace_back(CAST_I4(methodHandleInfo->referenceKind));
        linkMethodHandleParam.emplace_back(methodHandleClassMirrorOop);
        linkMethodHandleParam.emplace_back(methodHandleMemberNameOop);
        linkMethodHandleParam.emplace_back(type);

        const auto result = frame.runMethodManual(*linkMethodHandleConstantMethod, linkMethodHandleParam);
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(std::get<0>(result).refVal);
    }

    InstanceOop *createCallSite(
        Frame &frame, 
        InstanceOop *methodHandle, 
        cview invokeName, 
        cview invokeDescriptor, 
        InstanceClass *callerClass,
        const std::vector<u2> &bootstrapArguments
    ) {
        const auto &constantPool = frame.klass.constantPool;

        const auto methodHandleNativesClass = frame.mem.getInstanceClass("java/lang/invoke/MethodHandleNatives");
        const auto linkCallSiteImplMethod = methodHandleNativesClass->getMethod("linkCallSiteImpl" "(Ljava/lang/Class;Ljava/lang/invoke/MethodHandle;Ljava/lang/String;Ljava/lang/invoke/MethodType;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/invoke/MemberName;", true);

        const auto arraySize = bootstrapArguments.size();
        std::vector<ref> argResults;
        argResults.reserve(arraySize + 1);
        
        FOR_FROM_ZERO(arraySize) {
            const auto index = bootstrapArguments[i];
            const auto constantInfo = constantPool[index].get();
            const auto tagEnum = CAST_CONSTANT_TAG_ENUM(constantInfo->tag);
            ref argResult = nullptr;
            switch (tagEnum) {
                case ConstantTagEnum::CONSTANT_MethodHandle:
                    //argResult = createMethodHandle(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo), lookupOop);
                    argResult = createMethodHandle(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo), callerClass);
                    break;
                case ConstantTagEnum::CONSTANT_MethodType:
                    argResult = createMethodType(frame, CAST_CONSTANT_METHOD_TYPE_INFO(constantInfo));
                    break;
                case ConstantTagEnum::CONSTANT_String: {
                    const auto strVal = getConstantStringFromPoolByIndexInfo(constantPool, CAST_CONSTANT_STRING_INFO(constantInfo)->index);
                    argResult = frame.mem.getInternString(strVal);
                    break;
                }
                case ConstantTagEnum::CONSTANT_Class: {
                    const auto className = getConstantStringFromPool(constantPool, CAST_CONSTANT_CLASS_INFO(constantInfo)->index);
                    argResult = frame.mem.getClass(className)->getMirror(&frame);
                    break;
                }
                case ConstantTagEnum::CONSTANT_Integer:
                    argResult = frame.mem.newIntegerOop((CAST_CONSTANT_INTEGER_INFO(constantInfo))->value);
                    break;
                case ConstantTagEnum::CONSTANT_Float:
                    argResult = frame.mem.newFloatOop((CAST_CONSTANT_FLOAT_INFO(constantInfo))->value);
                    break;
                case ConstantTagEnum::CONSTANT_Long:
                    argResult = frame.mem.newLongOop((CAST_CONSTANT_LONG_INFO(constantInfo))->value);
                    break;
                case ConstantTagEnum::CONSTANT_Double:
                    argResult = frame.mem.newDoubleOop((CAST_CONSTANT_DOUBLE_INFO(constantInfo))->value);
                    break;
                default:
                    panic("error constant type");
                    break;
            }
            if (frame.markThrow) {
                return nullptr;
            }
            argResults.emplace_back(argResult);
        }
        
        const auto argObjArrayOop = frame.mem.newObjectObjArrayOop(arraySize);
        //这个oop分配必须放在这里 之前是放在for循环上面的 因为for循环中有函数调用 就被gc掉了
        FOR_FROM_ZERO(arraySize) {
            argObjArrayOop->data[i] = argResults[i];
        }

        const auto appendixResultArrayOop = frame.mem.newObjectObjArrayOop(1);
        std::vector<Slot> linkCallSiteParams;
        linkCallSiteParams.reserve(6);
        linkCallSiteParams.emplace_back(callerClass->getMirror(&frame));
        linkCallSiteParams.emplace_back(methodHandle);
        linkCallSiteParams.emplace_back(frame.mem.getInternString(invokeName));
        linkCallSiteParams.emplace_back(createMethodType(frame, invokeDescriptor));
        linkCallSiteParams.emplace_back(argObjArrayOop);
        linkCallSiteParams.emplace_back(appendixResultArrayOop);
        frame.runMethodManual(*linkCallSiteImplMethod, linkCallSiteParams);
        if (frame.markThrow) {
            return nullptr;
        }

        const auto finalMethodHandleOop = CAST_INSTANCE_OOP(appendixResultArrayOop->data[0]);
        const auto finalMethodHandClass = finalMethodHandleOop->getInstanceClass();
        const auto invokeMethod = finalMethodHandClass->getMethod("invoke", METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR, false);
        const auto [paramType, returnType] = parseMethodDescriptor(invokeDescriptor); 

        //在调用invokedynamic指令之前 最后一步invoke方法的参数已经被push到了操作栈上
        //参数的Slot数量可以通过invokeDescriptor来解析得到
        //但现在的参数栈有个问题 在调用methodHandle的invoke方法的第一个参数是一个MethodHandle对象
        //而现在这个对象才被我们通过dynamicInvoker生成出来 而因为invoke方法的特殊性(方法descriptor和实际参数不一致)
        //所以最好使用runMethodInner来调用 就跟解释器里的invokeMethodHandle里一样
        //runMethodInner会直接将栈来当做Local区 所以现在需要调整好栈中数据后 直接调用 runMethodInner
        //当前  Stack:[ x1, x2, x3, ... param1, param2, param3 ]  
        //调整  Stack:[ x1, x2, x3, ... methodHadnleOop, param1, param2, param3 ]
        //先把参数pop到一个vector 再将 methodHadnleOop 添加到vector 再反转vector后push回去
        //就得到了正确的栈结构 调用runMethodInner后返回值会被push到当前frame的操作栈中 pop到结果即可

        size_t paramSize = 1; //finalMethodHandleOop
        std::vector<std::tuple<Slot, SlotTypeEnum>> invokeParam;
        if (!paramType.empty()) {
            for (i4 i = CAST_I4(paramType.size()) - 1; i >= 0; --i) {
                const auto &paramClassName = paramType[i];
                if (isWideClassName(paramClassName)) {
                    frame.popWithSlotType();
                    invokeParam.emplace_back(frame.popWithSlotType());
                    paramSize += 2;
                } else {
                    invokeParam.emplace_back(frame.popWithSlotType());
                    paramSize += 1;
                }
            }
        }

        invokeParam.emplace_back(Slot(finalMethodHandleOop), SlotTypeEnum::REF);
        std::ranges::reverse(invokeParam);

        for (const auto &[val, type] : invokeParam) {
            frame.push(val, type);
        }

        frame.runMethodInner(*invokeMethod, paramSize);
        if (frame.markThrow) {
            return nullptr;
        }
        return CAST_INSTANCE_OOP(frame.pop().refVal);
    }

    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx) {   
        ThreadSafeGuard threadGuard(frame.thread);

        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto callerClass = &frame.method.klass;
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[invokeDynamicIdx].get());

        const auto [invokeName, invokeDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);
        const auto bootstrapMethodAttr = methodClass.getBootstrapMethodAttr()->bootstrapMethods[invokeDynamicInfo->bootstrapMethodAttrIndex].get();
        const auto methodHandleInfo = CAST_CONSTANT_METHOD_HANDLE_INFO(constantPool[bootstrapMethodAttr->bootstrapMethodRef].get());

        const auto bootstrapMethodHandle = createMethodHandle(frame, methodHandleInfo, callerClass);
        if (frame.markThrow) {
            return;
        }

        const auto callSiteObj = createCallSite(frame, bootstrapMethodHandle, invokeName, invokeDescriptor, callerClass, bootstrapMethodAttr->bootstrapArguments);
        if (frame.markThrow) {
            return;
        }

        frame.pushRef(callSiteObj);
    }

    cstring methodHandleGetDescriptor(Class *clazz, InstanceOop *type, cview name) {
        cstring descriptor{};
        if (isMethodHandleInvoke(clazz->getClassName(), name)) {
            descriptor = METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR;
        } else {
            const auto typeClass = type->getClass();
            const auto typeClassName = typeClass->getClassName();
            if (typeClassName == JAVA_LANG_INVOKE_METHOD_TYPE_NAME) {
                descriptor += "(";
                const auto ptypes = CAST_OBJ_ARRAY_OOP(type->getFieldValue("ptypes" "[Ljava/lang/Class;").refVal);
                for (size_t i = 0; i < ptypes->getDataLength(); ++i) {
                    const auto classMirrorOop = CAST_MIRROR_OOP(ptypes->data[i]);
                    const auto mirrorClass = classMirrorOop->getMirrorClass();
                    descriptor += mirrorClass->getClassDescriptor();
                }
                descriptor += ")";
                const auto rtype = CAST_MIRROR_OOP(type->getFieldValue("rtype" "Ljava/lang/Class;").refVal);
                descriptor += rtype->getMirrorClass()->getClassDescriptor();
            } else if (typeClassName == JAVA_LANG_CLASS_NAME) {
                const auto mirrorClass = CAST_MIRROR_OOP(type)->getMirrorClass();
                descriptor = mirrorClass->getClassDescriptor();
            } else {
                panic("error typeClassName");
            }
        }
        return descriptor;
    }

    std::tuple<
            InstanceClass *,
            cstring,
            InstanceOop *,
            i4,
            MethodHandleEnum,
            bool,
            cstring
    > methodHandleGetFieldFromMemberName(InstanceOop *memberName) {
        const auto klass = GET_MIRROR_INSTANCE_CLASS(memberName->getFieldValue("clazz" "Ljava/lang/Class;").refVal);
        const auto name = VMStringHelper::getJavaString(CAST_INSTANCE_OOP(memberName->getFieldValue("name" "Ljava/lang/String;").refVal));
        const auto type = CAST_INSTANCE_OOP(memberName->getFieldValue("type" "Ljava/lang/Object;").refVal);
        const auto flags = memberName->getFieldValue("flags" "I").i4Val;
        const auto kind = static_cast<MethodHandleEnum>((flags >> MN_REFERENCE_KIND_SHIFT) & MN_REFERENCE_KIND_MASK);
        const auto isStatic = kind == MethodHandleEnum::REF_invokeStatic;
        const auto descriptor = methodHandleGetDescriptor(klass, type, name);
        return std::make_tuple(klass, name, type, flags, kind, isStatic, descriptor);
    }

}
