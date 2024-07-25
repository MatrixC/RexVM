#include "invoke_dynamic.hpp"
#include "vm.hpp"
#include "memory.hpp"
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

    InstanceOop *createLookup(Frame &frame, const InstanceClass *targetLookupClass) {
        const auto lookupClass = frame.getCurrentClassLoader()->getInstanceClass("java/lang/invoke/MethodHandles$Lookup");
        const auto lookupOop = frame.vm.oopManager->newInstance(lookupClass);
        lookupOop->setFieldValue("allowedModes", "I", Slot(CAST_I4(15)));
        lookupOop->setFieldValue("lookupClass", "Ljava/lang/Class;", Slot(targetLookupClass->getMirrorOop()));
        return lookupOop;
    }

    //MethodType.fromMethodDescriptorString
    InstanceOop *createMethodeType_(Frame &frame, const cstring &methodDescriptor) {
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto [paramType, returnTypeType] = parseMethodDescriptor(methodDescriptor);
        const auto returnTypeParam = Slot(classLoader->getClass(returnTypeType)->getMirrorOop());
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = oopManager->newObjArrayOop(classArrayClass, paramType.size());

        size_t i = 0;
        for (const auto &className: paramType) {
            classArrayOop->data[i++] = classLoader->getClass(className)->getMirrorOop();
        }

        const auto methodTypeClass = classLoader->getInstanceClass("java/lang/invoke/MethodType");
        const auto methodTypeOop = oopManager->newInstance(methodTypeClass);
        methodTypeOop->setFieldValue("rtype", "Ljava/lang/Class;", returnTypeParam);
        methodTypeOop->setFieldValue("ptypes", "[Ljava/lang/Class;", Slot(classArrayOop));

        return methodTypeOop;
    }

    InstanceOop *createMethodeType_(Frame &frame, ConstantMethodTypeInfo *methodTypeInfo) {
        const auto &constantPool = frame.klass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto methodDescriptor = getConstantStringFromPool(constantPool, methodTypeInfo->descriptorIndex);
        return createMethodeType_(frame, methodDescriptor);
    }

    InstanceOop *createMethodHandle_(Frame &frame, ConstantMethodHandleInfo *methodHandleInfo, InstanceOop *lookupOop) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        const auto methodHandleKind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        const auto runtimeMethodHandleClass = classLoader->getInstanceClass("Rex");
        const auto createMethodHandleMethod = 
            runtimeMethodHandleClass->getMethod(
                "createMethodHandle", 
                "(Ljava/lang/invoke/MethodHandles$Lookup;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/invoke/MethodHandle;", 
                true
            );

        const auto [methodHandleSlot, _] = frame.runMethodManual(*createMethodHandleMethod, { 
            Slot(lookupOop),
            Slot(CAST_I4(methodHandleInfo->referenceKind)),
            Slot(stringPool->getInternString(getJavaClassName(methodHandleClassName))), 
            Slot(stringPool->getInternString(methodHandleMemberName)), 
            Slot(stringPool->getInternString(methodHandleMemberDescriptor)) 
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(methodHandleSlot.refVal);
    }

    ObjArrayOop *createBootstrapArgs(Frame &frame, const std::vector<u2> &bootstrapArguments, InstanceOop *lookupOop) {
        const auto &vm = frame.vm;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &constantPool = frame.klass.constantPool;
        const auto arraySize = bootstrapArguments.size();
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = vm.oopManager->newObjArrayOop(classArrayClass, arraySize);

        for (size_t i = 0; i < bootstrapArguments.size(); ++i) {
            const auto index = bootstrapArguments[i];
            const auto constantInfo = constantPool[index].get();
            const auto tagEnum = CAST_CONSTANT_TAG_ENUM(constantInfo->tag);
            ref argResult = nullptr;
            switch (tagEnum) {
                case ConstantTagEnum::CONSTANT_MethodHandle:
                    argResult = createMethodHandle_(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo), lookupOop);
                break;

                case ConstantTagEnum::CONSTANT_MethodType:
                    argResult = createMethodeType_(frame, CAST_CONSTANT_METHOD_TYPE_INFO(constantInfo));
                break;
            }
            if (frame.markThrow) {
                return nullptr;
            }
            classArrayOop->data[i] = argResult;
        }

        return classArrayOop;
    }

    ObjArrayOop *createBootstrapArgs_(
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

        const auto bootstrapArgsSize = bootstrapArguments.size();
        const auto fixedArgSize = 3;
        const auto arraySize = bootstrapArgsSize + fixedArgSize;
        const auto classArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_CLASS_NAME);
        const auto classArrayOop = vm.oopManager->newObjArrayOop(classArrayClass, arraySize);
        classArrayOop->data[0] = lookupOop;
        classArrayOop->data[1] = stringPool->getInternString(invokeName);
        classArrayOop->data[2] = createMethodeType_(frame, invokeDescriptor);


        for (size_t i = 0; i < bootstrapArguments.size(); ++i) {
            const auto index = bootstrapArguments[i];
            const auto constantInfo = constantPool[index].get();
            const auto tagEnum = CAST_CONSTANT_TAG_ENUM(constantInfo->tag);
            ref argResult = nullptr;
            switch (tagEnum) {
                case ConstantTagEnum::CONSTANT_MethodHandle:
                    argResult = createMethodHandle_(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo), lookupOop);
                break;

                case ConstantTagEnum::CONSTANT_MethodType:
                    argResult = createMethodeType_(frame, CAST_CONSTANT_METHOD_TYPE_INFO(constantInfo));
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
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto runtimeMethodHandleClass = classLoader->getInstanceClass("Rex");
        const auto createCallSite = 
            runtimeMethodHandleClass->getMethod(
                "createCallSite", 
                "(Ljava/lang/invoke/MethodHandles$Lookup;Ljava/lang/invoke/MethodHandle;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;", 
                true
            );

        const auto [callSiteSlot, _] = frame.runMethodManual(*createCallSite, { 
            Slot(lookupOop),
            Slot(methodHandle), 
            Slot(stringPool->getInternString(invokeName)), 
            Slot(stringPool->getInternString(invokeDescriptor)),
            Slot(bootstrapArgs)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(callSiteSlot.refVal);
    }

    InstanceOop *createCallSite_(Frame &frame, InstanceOop *methodHandle, const cstring &invokeName, const cstring &invokeDescriptor, ObjArrayOop *bootstrapArgs, InstanceOop *lookupOop) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

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
        const auto [dynamicInvokerSlot, _] = frame.runMethodManual(*dynamicInvokerMethod, { 
            Slot(callSiteOop)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        const auto finalMethodHandOop = CAST_INSTANCE_OOP(dynamicInvokerSlot.refVal);
        const auto finalMethodHandClass = finalMethodHandOop->getInstanceClass();
        const auto invokeMethod = finalMethodHandClass->getMethod("invoke", METHOD_HANDLE_INVOKE_ORIGIN_DESCRITPR, false);
        const auto [invokerSlot, _] = frame.runMethodManual(*invokeMethod, { 
            Slot(finalMethodHandOop)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(invokerSlot.refVal);
    }

    InstanceOop *createCallSite2(Frame &frame, InstanceOop *methodHandle, ObjArrayOop *bootstrapArgs, InstanceOop *lookupOop) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto runtimeMethodHandleClass = classLoader->getInstanceClass("Rex");
        const auto createCallSite = 
            runtimeMethodHandleClass->getMethod(
                "createCallSite2", 
                "(Ljava/lang/invoke/MethodHandle;[Ljava/lang/Object;)Ljava/lang/Object;", 
                true
            );

        const auto [callSiteSlot, _] = frame.runMethodManual(*createCallSite, { 
            Slot(methodHandle), 
            Slot(bootstrapArgs)
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(callSiteSlot.refVal);

    
    }


    void invokeDynmic_(Frame &frame, u2 invokeDynamicIdx) {
        auto &vm = frame.vm;
        const auto &oopManager = vm.oopManager;
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto &lookupClass = frame.method.klass;
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[invokeDynamicIdx].get());
        //=========================================================================================================
        //Key
        const auto [invokeName, invokeDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);
        const auto [paramType, _] = parseMethodDescriptor(invokeDescriptor); 
        //1是因为第一个参数为MethodHandle Object
        // size_t popLength = 0;
        // for (const auto &paramClassName : paramType) {
        //     if (isWideClassName(paramClassName)) {
        //         popLength += 2;
        //     } else {
        //         popLength += 1;
        //     }
        // }

        const auto bootstrapMethodAttr = methodClass.getBootstrapMethodAttr()->bootstrapMethods[invokeDynamicInfo->bootstrapMethodAttrIndex].get();
        const auto methodHandleInfo = CAST_CONSTANT_METHOD_HANDLE_INFO(constantPool[bootstrapMethodAttr->bootstrapMethodRef].get());

        const auto lookupOop = createLookup(frame, &lookupClass);

        const auto bootstrapMethodHandle = createMethodHandle_(frame, methodHandleInfo, lookupOop);
        if (frame.markThrow) {
            return;
        }
        const auto bootstrapArguments = createBootstrapArgs(frame, bootstrapMethodAttr->bootstrapArguments, lookupOop);
        if (frame.markThrow) {
            return;
        }
        const auto callSiteObj = createCallSite(frame, bootstrapMethodHandle, invokeName, invokeDescriptor, bootstrapArguments, lookupOop);
        if (frame.markThrow) {
            return;
        }
        // const auto bootstrapArguments = createBootstrapArgs_(frame, bootstrapMethodAttr->bootstrapArguments, lookupOop, invokeName, invokeDescriptor);
        // if (frame.markThrow) {
        //     return;
        // }
        // const auto callSiteObj = createCallSite_(frame, bootstrapMethodHandle, invokeName, invokeDescriptor, bootstrapArguments, lookupOop);
        // if (frame.markThrow) {
        //     return;
        // }
        // const auto callSiteObj = createCallSite2(frame, bootstrapMethodHandle, bootstrapArguments, lookupOop);
        // if (frame.markThrow) {
        //     return;
        // }


        frame.pushRef(callSiteObj);
    }

}
