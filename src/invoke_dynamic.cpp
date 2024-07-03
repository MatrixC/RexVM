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

    InstanceOop *createLookup(VM &vm, Frame &frame) {
        //TODO Opt key slot id
        const auto methodHandlesClass = vm.bootstrapClassLoader->getInstanceClass("java/lang/invoke/MethodHandles");
        const auto lookupMethod = methodHandlesClass->getMethod("lookup", "()Ljava/lang/invoke/MethodHandles$Lookup;", true);
        const auto lookupOop = frame.runMethodGetReturn(*lookupMethod, {});
        return static_cast<InstanceOop *>(lookupOop.refVal);
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
        auto &vm = frame.vm;
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
        Oop *result;
        switch (kind) {
            case MethodHandleEnum::REF_getField:
                method = lookupOopClass->getMethod("findGetter", refFieldDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                }).refVal;
                break;

            case MethodHandleEnum::REF_getStatic:
                method = lookupOopClass->getMethod("findStaticGetter", refFieldDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                }).refVal;
            break;

            case MethodHandleEnum::REF_putField:
                method = lookupOopClass->getMethod("findSetter", refFieldDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                }).refVal;
            break;

            case MethodHandleEnum::REF_putStatic:
                method = lookupOopClass->getMethod("findStaticSetter", refFieldDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(classLoader->getClass(methodHandleMemberDescriptor)->getMirrorOop())
                }).refVal;
            break;

            case MethodHandleEnum::REF_invokeVirtual:
                method = lookupOopClass->getMethod("findVirtual", refMethodDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                }).refVal;
            break;

            case MethodHandleEnum::REF_invokeStatic:
                method = lookupOopClass->getMethod("findStatic", refMethodDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                }).refVal;
            break;

            case MethodHandleEnum::REF_invokeSpecial:
                method = lookupOopClass->getMethod("findSpecial", refMethodDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                }).refVal;
            break;

            case MethodHandleEnum::REF_newInvokeSpecial:
                method = lookupOopClass->getMethod("findSpecial", refMethodDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                }).refVal;
            break;

            case MethodHandleEnum::REF_invokeInterface:
                method = lookupOopClass->getMethod("findVirtual", refMethodDescriptor, false);
                result = frame.runMethodGetReturn(*method, {
                        lookupOopThisParam,
                        methodHandleClassParam,
                        methodHandleMemberNameParam,
                        Slot(createMethodType(vm, methodHandleMemberDescriptor))
                }).refVal;
            break;

            default:
                panic("createMethodHandles error: error kind");
        }

        return static_cast<InstanceOop *>(result);
    }

    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx) {
        auto &vm = frame.vm;
        const auto &oopManager = vm.oopManager;
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto invokeDynamicInfo = static_cast<ConstantInvokeDynamicInfo *>(constantPool[invokeDynamicIdx].get());
        //=========================================================================================================
        //Key
        const auto [invokeName, invokeDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);

        const auto bootstrapMethodAttr = methodClass.getBootstrapMethodAttr()->bootstrapMethods[invokeDynamicInfo->bootstrapMethodAttrIndex].get();
        const auto methodHandleInfo = static_cast<ConstantMethodHandleInfo *>(constantPool[bootstrapMethodAttr->bootstrapMethodRef].get());
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
            const auto argInfoType = static_cast<ConstantTagEnum>(argInfo->tag);
            switch (argInfoType) {
                case ConstantTagEnum::CONSTANT_String: {
                    const auto strVal = getConstantStringFromPoolByIndexInfo(constantPool, argIdx);
                    callSiteParam.emplace_back(vm.stringPool->getInternString(strVal));
                    break;
                }

                case ConstantTagEnum::CONSTANT_Class: {
                    const auto className = getConstantStringFromPool(constantPool, static_cast<ConstantClassInfo *>(argInfo)->index);
                    callSiteParam.emplace_back(vm.bootstrapClassLoader->getClass(className)->getMirrorOop());
                    break;
                }

                case ConstantTagEnum::CONSTANT_Integer:
                    callSiteParam.emplace_back(oopManager->newIntegerOop((static_cast<ConstantIntegerInfo *>(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Float:
                    callSiteParam.emplace_back(oopManager->newFloatOop((static_cast<ConstantFloatInfo *>(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Long:
                    callSiteParam.emplace_back(oopManager->newLongOop((static_cast<ConstantLongInfo *>(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_Double:
                    callSiteParam.emplace_back(oopManager->newDoubleOop((static_cast<ConstantDoubleInfo *>(argInfo))->value));
                break;

                case ConstantTagEnum::CONSTANT_MethodHandle:
                    callSiteParam.emplace_back(createMethodHandle(frame, static_cast<ConstantMethodHandleInfo *>(argInfo)));
                break;

                case ConstantTagEnum::CONSTANT_MethodType:
                    callSiteParam.emplace_back(createMethodType(vm, getConstantStringFromPool(constantPool, (static_cast<ConstantMethodTypeInfo *>(argInfo))->descriptorIndex)));
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

        frame.runMethod(*arrayListInitMethod, { Slot(arrayListOop) });
        for (const auto &item : callSiteParam) {
            frame.runMethodGetReturn(*arrayListAddMethod, { Slot(item) });
        }

        const auto invokeWithArgumentsMethod = methodHandleOop->getInstanceClass()->getMethod("invokeWithArguments", "(Ljava/util/List;)Ljava/lang/Object;", false);
        const auto callSiteOop = static_cast<InstanceOop *>(frame.runMethodGetReturn(*invokeWithArgumentsMethod, { Slot(methodHandleOop), Slot(arrayListOop)}).refVal);

        const auto [invokeParamType, _] = parseMethodDescriptor(invokeDescriptor);
        const auto dynamicInvokerMethod = callSiteOop->getInstanceClass()->getMethod("dynamicInvoker", "()Ljava/lang/invoke/MethodHandle;", false);
        const auto invokeMethodHandleOop = static_cast<InstanceOop *>(frame.runMethodGetReturn(*dynamicInvokerMethod, { Slot(callSiteOop) }).refVal);

        const auto invokeExactMethod = invokeMethodHandleOop->getInstanceClass()->getMethod("invokeExact", "([Ljava/lang/Object;)Ljava/lang/Object;", false);









    }



}