#include "method_handle.hpp"
#include "memory.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "string_pool.hpp"
#include "oop.hpp"
#include "class.hpp"
#include "class_loader.hpp"
#include "frame.hpp"
#include "utils/descriptor_parser.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    bool isMethodHandleInvoke(const cstring &className, const cstring &memberName) {
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
    InstanceOop *createMethodType(Frame &frame, const cstring &methodDescriptor) {
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto [paramType, returnTypeType] = parseMethodDescriptor(methodDescriptor);
        const auto returnTypeOop = classLoader->getClass(returnTypeType)->getMirrorOop();
        const auto classArrayOop = oopManager->newClassObjArrayOop(paramType.size());

        size_t i = 0;
        for (const auto &className: paramType) {
            classArrayOop->data[i++] = classLoader->getClass(className)->getMirrorOop();
        }
        const auto runtimeMethodTypeClass = classLoader->getInstanceClass("java/lang/invoke/MethodType");
        const auto makeImplMethod = runtimeMethodTypeClass->getMethod("makeImpl", "(Ljava/lang/Class;[Ljava/lang/Class;Z)Ljava/lang/invoke/MethodType;", true);

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

    InstanceOop *createMethodType(Frame &frame, ConstantMethodTypeInfo *methodTypeInfo) {
        const auto &constantPool = frame.klass.constantPool;
        const auto methodDescriptor = getConstantStringFromPool(constantPool, methodTypeInfo->descriptorIndex);
        return createMethodType(frame, methodDescriptor);
    }

    InstanceOop *createMethodHandle(
        Frame &frame, 
        ConstantMethodHandleInfo *methodHandleInfo, 
        InstanceClass *callerClass
    ) {
        const auto &vm = frame.vm;
        const auto &methodClass = frame.klass;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &constantPool = methodClass.constantPool;

        const auto methodHandleNativesClass = frame.getCurrentClassLoader()->getInstanceClass("java/lang/invoke/MethodHandleNatives");
        const auto linkMethodHandleConstantMethod = methodHandleNativesClass->getMethod("linkMethodHandleConstant", "(Ljava/lang/Class;ILjava/lang/Class;Ljava/lang/String;Ljava/lang/Object;)Ljava/lang/invoke/MethodHandle;", true);

        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        const auto kind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        const auto methodHandleClass = classLoader->getInstanceClass(methodHandleClassName);
        const auto methodHandleClassMirrorOop = methodHandleClass->getMirrorOop();
        const auto methodHandleMemberNameOop = vm.stringPool->getInternString(methodHandleMemberName);

        Slot type;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
            case MethodHandleEnum::REF_getStatic:
            case MethodHandleEnum::REF_putField:
            case MethodHandleEnum::REF_putStatic:
                type = Slot(classLoader->getClass(getClassNameByFieldDescriptor(methodHandleMemberDescriptor))->getMirrorOop());
            break;

            default:
                type = Slot(createMethodType(frame, methodHandleMemberDescriptor));
            break;
        }

        std::vector<Slot> linkMethodHandleParam;
        linkMethodHandleParam.reserve(5);
        linkMethodHandleParam.emplace_back(callerClass->getMirrorOop());
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
        const cstring &invokeName, 
        const cstring &invokeDescriptor, 
        InstanceClass *callerClass,
        const std::vector<u2> &bootstrapArguments
    ) {
        const auto &vm = frame.vm;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &constantPool = frame.klass.constantPool;
        const auto &stringPool = frame.vm.stringPool;
        const auto &oopManager = vm.oopManager;

        const auto methodHandleNativesClass = frame.getCurrentClassLoader()->getInstanceClass("java/lang/invoke/MethodHandleNatives");
        const auto linkCallSiteImplMethod = methodHandleNativesClass->getMethod("linkCallSiteImpl", "(Ljava/lang/Class;Ljava/lang/invoke/MethodHandle;Ljava/lang/String;Ljava/lang/invoke/MethodType;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/invoke/MemberName;", true);

        const auto arraySize = bootstrapArguments.size();
        const auto argObjArrayOop = oopManager->newObjectObjArrayOop(arraySize);
        for (size_t i = 0; i < bootstrapArguments.size(); ++i) {
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
                    argResult = stringPool->getInternString(strVal);
                    break;
                }
                case ConstantTagEnum::CONSTANT_Class: {
                    const auto className = getConstantStringFromPool(constantPool, CAST_CONSTANT_CLASS_INFO(constantInfo)->index);
                    argResult = classLoader->getClass(className)->getMirrorOop();
                    break;
                }
                case ConstantTagEnum::CONSTANT_Integer:
                    argResult = oopManager->newIntegerOop((CAST_CONSTANT_INTEGER_INFO(constantInfo))->value);
                break;
                case ConstantTagEnum::CONSTANT_Float:
                    argResult = oopManager->newFloatOop((CAST_CONSTANT_FLOAT_INFO(constantInfo))->value);
                break;
                case ConstantTagEnum::CONSTANT_Long:
                    argResult = oopManager->newLongOop((CAST_CONSTANT_LONG_INFO(constantInfo))->value);
                break;
                case ConstantTagEnum::CONSTANT_Double:
                    argResult = oopManager->newDoubleOop((CAST_CONSTANT_DOUBLE_INFO(constantInfo))->value);
                break;
                default:
                    panic("error constant type");
                break;
            }
            if (frame.markThrow) {
                return nullptr;
            }
            argObjArrayOop->data[i] = argResult;
        }

        
        const auto appendixResultArrayOop = oopManager->newObjectObjArrayOop(1);
        std::vector<Slot> linkCallSiteParms;
        linkCallSiteParms.reserve(5);
        linkCallSiteParms.emplace_back(callerClass->getMirrorOop());
        linkCallSiteParms.emplace_back(methodHandle);
        linkCallSiteParms.emplace_back(frame.vm.stringPool->getInternString(invokeName));
        linkCallSiteParms.emplace_back(createMethodType(frame, invokeDescriptor));
        linkCallSiteParms.emplace_back(argObjArrayOop);
        linkCallSiteParms.emplace_back(appendixResultArrayOop);
        frame.runMethodManual(*linkCallSiteImplMethod, linkCallSiteParms);
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
            for (i4 i = paramType.size() - 1; i >= 0; --i) {
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

        invokeParam.emplace_back(std::make_tuple(Slot(finalMethodHandleOop), SlotTypeEnum::REF));
        std::reverse(invokeParam.begin(), invokeParam.end());

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

    cstring methodHandleGetDescriptor(Class *clazz, InstanceOop *type, const cstring &name) {
        cstring descriptor{};
        if (isMethodHandleInvoke(clazz->name, name)) {
            descriptor = METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR;
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

}
