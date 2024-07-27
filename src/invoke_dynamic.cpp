#include "invoke_dynamic.hpp"
#include "memory.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "constant_pool.hpp"
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

    InstanceOop *createLookup(Frame &frame, const InstanceClass *targetLookupClass) {
        const auto lookupClass = frame.getCurrentClassLoader()->getInstanceClass("java/lang/invoke/MethodHandles$Lookup");
        const auto lookupOop = frame.vm.oopManager->newInstance(lookupClass);
        const auto allowsModes = -1; //-1: TRUSTED, 15: ALL_MODES 都可以
        lookupOop->setFieldValue("allowedModes", "I", Slot(CAST_I4(allowsModes)));
        lookupOop->setFieldValue("lookupClass", "Ljava/lang/Class;", Slot(targetLookupClass->getMirrorOop()));
        return lookupOop;
    }

    //代替MethodType.fromMethodDescriptorString
    InstanceOop *createMethodType(Frame &frame, const cstring &methodDescriptor) {
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto [paramType, returnTypeType] = parseMethodDescriptor(methodDescriptor);
        const auto returnTypeOop = classLoader->getClass(returnTypeType)->getMirrorOop();
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = oopManager->newObjArrayOop(classArrayClass, paramType.size());

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
        InstanceOop *lookupOop
    ) {
        auto &vm = frame.vm;
        const auto &methodClass = frame.klass;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &constantPool = methodClass.constantPool;
        const auto lookupOopClass = lookupOop->getInstanceClass();

        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        const auto kind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        const auto methodHandleClass = classLoader->getInstanceClass(methodHandleClassName);
        const auto methodHandleClassMirrorOop = methodHandleClass->getMirrorOop();
        const auto methodHandleMemberNameOop = vm.stringPool->getInternString(methodHandleMemberName);

        const auto refFieldDescriptor = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;)Ljava/lang/invoke/MethodHandle;";
        const auto refMethodDescriptor = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/invoke/MethodType;)Ljava/lang/invoke/MethodHandle;";
        const auto refConstructorMethodDescriptor = "(Ljava/lang/Class;Ljava/lang/invoke/MethodType;)Ljava/lang/invoke/MethodHandle;";

        Method *method = nullptr;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
                method = lookupOopClass->getMethod("findGetter", refFieldDescriptor, false);
            break;
            case MethodHandleEnum::REF_getStatic:
                method = lookupOopClass->getMethod("findStaticGetter", refFieldDescriptor, false);
            break;
            case MethodHandleEnum::REF_putField:
                method = lookupOopClass->getMethod("findSetter", refFieldDescriptor, false);
            break;
            case MethodHandleEnum::REF_putStatic:
                method = lookupOopClass->getMethod("findStaticSetter", refFieldDescriptor, false);
            break;

            case MethodHandleEnum::REF_invokeVirtual:
            case MethodHandleEnum::REF_invokeInterface:
                method = lookupOopClass->getMethod("findVirtual", refMethodDescriptor, false);
            break;
            case MethodHandleEnum::REF_invokeStatic:
                method = lookupOopClass->getMethod("findStatic", refMethodDescriptor, false);
            break;
            case MethodHandleEnum::REF_invokeSpecial:
                method = lookupOopClass->getMethod("findSpecial", refMethodDescriptor, false);
            break;
            case MethodHandleEnum::REF_newInvokeSpecial:
                method = lookupOopClass->getMethod("findConstructor", refConstructorMethodDescriptor, false);
            break;

            default:
                panic("createMethodHandles error: error kind");
        }
        if (method == nullptr) {
            panic("can't find lookup method");
        }

        std::vector<Slot> params;
        params.emplace_back(lookupOop);
        params.emplace_back(methodHandleClassMirrorOop);

        std::tuple<Slot, SlotTypeEnum> result;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
            case MethodHandleEnum::REF_getStatic:
            case MethodHandleEnum::REF_putField:
            case MethodHandleEnum::REF_putStatic:
                params.emplace_back(methodHandleMemberNameOop);
                params.emplace_back(classLoader->getClass(getClassNameByFieldDescriptor(methodHandleMemberDescriptor))->getMirrorOop());
            break;

            case MethodHandleEnum::REF_invokeVirtual:
            case MethodHandleEnum::REF_invokeInterface:
            case MethodHandleEnum::REF_invokeStatic:
            case MethodHandleEnum::REF_invokeSpecial:
                params.emplace_back(methodHandleMemberNameOop);
                params.emplace_back(createMethodType(frame, methodHandleMemberDescriptor));
            break;

            case MethodHandleEnum::REF_newInvokeSpecial:
                params.emplace_back(createMethodType(frame, methodHandleMemberDescriptor));
            break;

            default:
                panic("createMethodHandles error: error kind");
        }
        result = frame.runMethodManual(*method, params);
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(std::get<0>(result).refVal);
    }

    ObjArrayOop *createBootstrapArgs(
        Frame &frame, 
        const std::vector<u2> &bootstrapArguments, 
        InstanceOop *lookupOop,
        const cstring &invokeName,
        const cstring &invokeDescriptor
    ) {
        const auto &vm = frame.vm;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &constantPool = frame.klass.constantPool;
        const auto &stringPool = frame.vm.stringPool;
        const auto &oopManager = vm.oopManager;

        const auto bootstrapArgsSize = bootstrapArguments.size();
        const auto fixedArgSize = 3;
        const auto arraySize = bootstrapArgsSize + fixedArgSize;
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = vm.oopManager->newObjArrayOop(classArrayClass, arraySize);
        //前三个参数固定 后面的参数由class文件中对应的attribute决定
        classArrayOop->data[0] = lookupOop;
        classArrayOop->data[1] = stringPool->getInternString(invokeName);
        classArrayOop->data[2] = createMethodType(frame, invokeDescriptor);


        for (size_t i = 0; i < bootstrapArguments.size(); ++i) {
            const auto index = bootstrapArguments[i];
            const auto constantInfo = constantPool[index].get();
            const auto tagEnum = CAST_CONSTANT_TAG_ENUM(constantInfo->tag);
            ref argResult = nullptr;
            switch (tagEnum) {
                case ConstantTagEnum::CONSTANT_MethodHandle:
                    argResult = createMethodHandle(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo), lookupOop);
                break;
                case ConstantTagEnum::CONSTANT_MethodType:
                    argResult = createMethodType(frame, CAST_CONSTANT_METHOD_TYPE_INFO(constantInfo));
                break;
                case ConstantTagEnum::CONSTANT_String: {
                    const auto strVal = getConstantStringFromPoolByIndexInfo(constantPool, CAST_CONSTANT_STRING_INFO(constantInfo)->index);
                    argResult = vm.stringPool->getInternString(strVal);
                    break;
                }
                case ConstantTagEnum::CONSTANT_Class: {
                    const auto className = getConstantStringFromPool(constantPool, CAST_CONSTANT_CLASS_INFO(constantInfo)->index);
                    argResult = vm.bootstrapClassLoader->getClass(className)->getMirrorOop();
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
            classArrayOop->data[i + fixedArgSize] = argResult;
        }

        return classArrayOop;
    }

    InstanceOop *createCallSite(Frame &frame, InstanceOop *methodHandle, const cstring &invokeName, const cstring &invokeDescriptor, ObjArrayOop *bootstrapArgs, InstanceOop *lookupOop) {
        const auto methodHandleClass = methodHandle->getInstanceClass();
        const auto invokeWithArgumentsMethod = methodHandleClass->getMethod("invokeWithArguments", "([Ljava/lang/Object;)Ljava/lang/Object;", false);
        const auto [invokeWithArgumentsSlot, _] = frame.runMethodManual(*invokeWithArgumentsMethod, { 
            Slot(methodHandle),
            Slot(bootstrapArgs)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        const auto callSiteOop = CAST_INSTANCE_OOP(invokeWithArgumentsSlot.refVal);
        const auto callSiteClass = callSiteOop->getInstanceClass();
        const auto dynamicInvokerMethod = callSiteClass->getMethod("dynamicInvoker", "()Ljava/lang/invoke/MethodHandle;", false);
        const auto [dynamicInvokerSlot, dynamicInvokerSlotType] = frame.runMethodManual(*dynamicInvokerMethod, { 
            Slot(callSiteOop)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        const auto finalMethodHandOop = CAST_INSTANCE_OOP(dynamicInvokerSlot.refVal);
        const auto finalMethodHandClass = finalMethodHandOop->getInstanceClass();
        const auto invokeMethod = finalMethodHandClass->getMethod("invoke", METHOD_HANDLE_INVOKE_ORIGIN_DESCRITPR, false);
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

        std::vector<std::tuple<Slot, SlotTypeEnum>> invokeParam;
        for (const auto &paramClassName : paramType) {
            if (isWideClassName(paramClassName)) {
                invokeParam.emplace_back(frame.popWithSlotType());
                invokeParam.emplace_back(frame.popWithSlotType());
            } else {
                invokeParam.emplace_back(frame.popWithSlotType());
            }
        }
        invokeParam.emplace_back(std::make_tuple(Slot(finalMethodHandOop), SlotTypeEnum::REF));
        std::reverse(invokeParam.begin(), invokeParam.end());

        for (const auto &[val, type] : invokeParam) {
            frame.push(val, type);
        }

        frame.runMethodInner(*invokeMethod, invokeParam.size());
        if (frame.markThrow) {
            return nullptr;
        }
        return CAST_INSTANCE_OOP(frame.pop().refVal);
    }

    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto &lookupClass = frame.method.klass;
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[invokeDynamicIdx].get());

        const auto [invokeName, invokeDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);
        const auto bootstrapMethodAttr = methodClass.getBootstrapMethodAttr()->bootstrapMethods[invokeDynamicInfo->bootstrapMethodAttrIndex].get();
        const auto methodHandleInfo = CAST_CONSTANT_METHOD_HANDLE_INFO(constantPool[bootstrapMethodAttr->bootstrapMethodRef].get());

        const auto lookupOop = createLookup(frame, &lookupClass);

        const auto bootstrapMethodHandle = createMethodHandle(frame, methodHandleInfo, lookupOop);
        if (frame.markThrow) {
            return;
        }

        const auto bootstrapArguments = createBootstrapArgs(frame, bootstrapMethodAttr->bootstrapArguments, lookupOop, invokeName, invokeDescriptor);
        if (frame.markThrow) {
            return;
        }
        const auto callSiteObj = createCallSite(frame, bootstrapMethodHandle, invokeName, invokeDescriptor, bootstrapArguments, lookupOop);
        if (frame.markThrow) {
            return;
        }

        frame.pushRef(callSiteObj);
    }

}
