#include "invoke_dynamic.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "constant_pool.hpp"
#include "oop.hpp"
#include "class.hpp"
#include "class_loader.hpp"
#include "frame.hpp"
#include "memory.hpp"
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



    InstanceOop *createLookup(VM &vm, Frame &frame) {
        //TODO Opt key slot id
        const auto methodHandlesClass = vm.bootstrapClassLoader->getInstanceClass("java/lang/invoke/MethodHandles");
        const auto lookupMethod = methodHandlesClass->getMethod("lookup", "()Ljava/lang/invoke/MethodHandles$Lookup;", true);
        const auto [lookupOop, _] = frame.runMethodManual(*lookupMethod, {});
        return CAST_INSTANCE_OOP(lookupOop.refVal);
    }

    InstanceOop *createMethodType(VM &vm, const cstring &methodDescriptor) {
        const auto classLoader = vm.bootstrapClassLoader.get();
        const auto [paramType, returnTypeType] = parseMethodDescriptor(methodDescriptor);
        const auto returnTypeParam = Slot(classLoader->getClass(returnTypeType)->getMirrorOop());
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = vm.oopManager->newObjArrayOop(classArrayClass, paramType.size());

        size_t i = 0;
        for (const auto &className: paramType) {
            classArrayOop->data[i++] = classLoader->getClass(className)->getMirrorOop();
        }

        const auto methodTypeClass = classLoader->getInstanceClass("java/lang/invoke/MethodType");
        const auto methodTypeOop = vm.oopManager->newInstance(methodTypeClass);
        methodTypeOop->setFieldValue("rtype", "Ljava/lang/Class;", returnTypeParam);
        methodTypeOop->setFieldValue("ptypes", "[Ljava/lang/Class;", Slot(classArrayOop));

        return methodTypeOop;
    }

    InstanceOop *createMethodHandle(Frame &frame, ConstantMethodHandleInfo *methodHandleInfo) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;

        //Key
        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        //Key
        const auto methodHandleKind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        return createMethodHandle(frame, methodHandleKind, methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor);        

    }
    

    InstanceOop *createMethodHandle(Frame &frame, MethodHandleEnum kind
        , const cstring &className, const cstring &methodHandleMemberName, const cstring &methodHandleMemberDescriptor) {
        auto &vm = frame.vm;
        const auto classLoader = vm.bootstrapClassLoader.get();
        const auto lookupOop = createLookup(vm, frame);
        const auto lookupOopClass = lookupOop->getInstanceClass();
        const auto lookupOopThisParam = Slot(lookupOop);

        const auto methodHandleClass = vm.bootstrapClassLoader->getInstanceClass(className);
        const auto methodHandleClassParam = Slot(methodHandleClass->getMirrorOop());
        const auto methodHandleMemberNameParam = Slot(vm.stringPool->getInternString(methodHandleMemberName));

        const auto refFieldDescriptor = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;)Ljava/lang/invoke/MethodHandle;";
        const auto refMethodDescriptor = "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/invoke/MethodType;)Ljava/lang/invoke/MethodHandle;";

        Method *method = nullptr;
        std::tuple<Slot, SlotTypeEnum> result;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
                method = lookupOopClass->getMethod("findGetter", refFieldDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                });
                break;

            case MethodHandleEnum::REF_getStatic:
                method = lookupOopClass->getMethod("findStaticGetter", refFieldDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                });
            break;

            case MethodHandleEnum::REF_putField:
                method = lookupOopClass->getMethod("findSetter", refFieldDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                });
            break;

            case MethodHandleEnum::REF_putStatic:
                method = lookupOopClass->getMethod("findStaticSetter", refFieldDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                });
            break;

            case MethodHandleEnum::REF_invokeVirtual:
                method = lookupOopClass->getMethod("findVirtual", refMethodDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                });
            break;

            case MethodHandleEnum::REF_invokeStatic:
                method = lookupOopClass->getMethod("findStatic", refMethodDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                });
            break;

            case MethodHandleEnum::REF_invokeSpecial:
                method = lookupOopClass->getMethod("findSpecial", refMethodDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                });
            break;

            case MethodHandleEnum::REF_newInvokeSpecial:
                method = lookupOopClass->getMethod("findSpecial", refMethodDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                });
            break;

            case MethodHandleEnum::REF_invokeInterface:
                method = lookupOopClass->getMethod("findVirtual", refMethodDescriptor, false);
                result = frame.runMethodManual(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                });
            break;

            default:
                panic("createMethodHandles error: error kind");
        }

        return CAST_INSTANCE_OOP(std::get<0>(result).refVal);
    }

    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx) {
        auto &vm = frame.vm;
        const auto &oopManager = vm.oopManager;
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[invokeDynamicIdx].get());
        //=========================================================================================================
        //Key
        const auto [invokeName, invokeDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);

        const auto bootstrapMethodAttr = methodClass.getBootstrapMethodAttr()->bootstrapMethods[invokeDynamicInfo->bootstrapMethodAttrIndex].get();
        const auto methodHandleInfo = CAST_CONSTANT_METHOD_HANDLE_INFO(constantPool[bootstrapMethodAttr->bootstrapMethodRef].get());
        //=========================================================================================================

        //Key
        const auto methodHandleOop = createMethodHandle(frame, methodHandleInfo);

        std::vector<Oop *> callSiteParam;
        callSiteParam.emplace_back(createLookup(vm, frame));
        callSiteParam.emplace_back(vm.stringPool->getInternString(invokeName));
        callSiteParam.emplace_back(createMethodType(vm, invokeDescriptor));
        for (size_t i = 0; i < bootstrapMethodAttr->numberOfBootstrapArguments; ++i) {
            const auto argIdx = bootstrapMethodAttr->bootstrapArguments[i];
            const auto argInfo = constantPool[argIdx].get();
            const auto argInfoType = CAST_CONSTANT_TAG_ENUM(argInfo->tag);
            switch (argInfoType) {
                case ConstantTagEnum::CONSTANT_String: {
                    const auto strVal = getConstantStringFromPoolByIndexInfo(constantPool, argIdx);
                    callSiteParam.emplace_back(vm.stringPool->getInternString(strVal));
                    break;
                }

                case ConstantTagEnum::CONSTANT_Class: {
                    const auto className = getConstantStringFromPool(constantPool, CAST_CONSTANT_CLASS_INFO(argInfo)->index);
                    callSiteParam.emplace_back(vm.bootstrapClassLoader->getClass(className)->getMirrorOop());
                    break;
                }

                case ConstantTagEnum::CONSTANT_Integer:
                    callSiteParam.emplace_back(oopManager->newIntegerOop((CAST_CONSTANT_INTEGER_INFO(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Float:
                    callSiteParam.emplace_back(oopManager->newFloatOop((CAST_CONSTANT_FLOAT_INFO(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Long:
                    callSiteParam.emplace_back(oopManager->newLongOop((CAST_CONSTANT_LONG_INFO(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Double:
                    callSiteParam.emplace_back(oopManager->newDoubleOop((CAST_CONSTANT_DOUBLE_INFO(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_MethodHandle:
                    callSiteParam.emplace_back(createMethodHandle(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(argInfo)));
                break;

                case ConstantTagEnum::CONSTANT_MethodType:
                    callSiteParam.emplace_back(createMethodType(vm, getConstantStringFromPool(constantPool, (CAST_CONSTANT_METHOD_TYPE_INFO(argInfo))->descriptorIndex)));
                break;

                default:
                    panic("invokedynamic error: error ArgInfoType");
                break;
            }
        }

        const auto arrayListClass = vm.bootstrapClassLoader->getInstanceClass("java/util/ArrayList");
        const auto arrayListOop = oopManager->newInstance(arrayListClass);
        const auto arrayListInitMethod = arrayListClass->getMethod("<init>", "()V", false);
        const auto arrayListAddMethod = arrayListClass->getMethod("add", "(Ljava/lang/Object;)Z", false);

        frame.runMethodManual(*arrayListInitMethod, { Slot(arrayListOop) });
        for (const auto &item : callSiteParam) {
            frame.runMethodManual(*arrayListAddMethod, { Slot(item) });
        }

        const auto invokeWithArgumentsMethod = methodHandleOop->getInstanceClass()->getMethod("invokeWithArguments", "(Ljava/util/List;)Ljava/lang/Object;", false);


        const auto callSiteOop = CAST_INSTANCE_OOP(std::get<0>(frame.runMethodManual(*invokeWithArgumentsMethod, { Slot(methodHandleOop), Slot(arrayListOop)})).refVal);
        const auto [invokeParamType, _] = parseMethodDescriptor(invokeDescriptor);
        const auto dynamicInvokerMethod = callSiteOop->getInstanceClass()->getMethod("dynamicInvoker", "()Ljava/lang/invoke/MethodHandle;", false);
        const auto invokeMethodHandleOop = CAST_INSTANCE_OOP(std::get<0>(frame.runMethodManual(*dynamicInvokerMethod, { Slot(callSiteOop) })).refVal);

        const auto invokeExactMethod = invokeMethodHandleOop->getInstanceClass()->getMethod("invokeExact", "([Ljava/lang/Object;)Ljava/lang/Object;", false);
        (void)invokeExactMethod;



    }



}