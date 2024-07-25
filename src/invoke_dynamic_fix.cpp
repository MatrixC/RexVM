#include "invoke_dynamic.hpp"
#include "vm.hpp"
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

    // InstanceOop *createLookup(Frame &frame, InstanceClass *lookupClass) {
    //     frame.getCurrentClassLoader()->getInstanceClass("");

    // }

    InstanceOop *createMethodHandle_(Frame &frame, ConstantMethodHandleInfo *methodHandleInfo) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto [methodHandleClassName, methodHandleMemberName, methodHandleMemberDescriptor] = 
            getConstantStringFromPoolByClassNameType(constantPool, methodHandleInfo->referenceIndex);
        const auto methodHandleKind = static_cast<MethodHandleEnum>(methodHandleInfo->referenceKind);

        const auto runtimeMethodHandleClass = classLoader->getInstanceClass("Rex");
        const auto createMethodHandleMethod = runtimeMethodHandleClass->getMethod("createMethodHandle", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/invoke/MethodHandle;", true);


        const auto [methodHandleSlot, _] = frame.runMethodManual(*createMethodHandleMethod, { 
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

    InstanceOop *createMethodeType_(Frame &frame, ConstantMethodTypeInfo *methodTypeInfo) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto methodTypeDescriptor = getConstantStringFromPool(constantPool, methodTypeInfo->descriptorIndex);

        const auto runtimeMethodTypeClass = classLoader->getInstanceClass("java/lang/invoke/MethodType");
        const auto fromMethodDescriptorStringMethod = runtimeMethodTypeClass->getMethod("fromMethodDescriptorString", "(Ljava/lang/String;Ljava/lang/ClassLoader;)Ljava/lang/invoke/MethodType;", true);

        const auto [methodTypeSlot, _] = frame.runMethodManual(*fromMethodDescriptorStringMethod, { 
            Slot(stringPool->getInternString(methodTypeDescriptor)), 
            Slot(nullptr) 
        });
        if (frame.markThrow) {
            return nullptr;
        }

        return CAST_INSTANCE_OOP(methodTypeSlot.refVal);
    }

    ObjArrayOop *createBootstrapArgs(Frame &frame, const std::vector<u2> &bootstrapArguments) {
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
                    argResult = createMethodHandle_(frame, CAST_CONSTANT_METHOD_HANDLE_INFO(constantInfo));
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

    InstanceOop *createCallSite(Frame &frame, InstanceOop *methodHandle, const cstring &invokeName, const cstring &invokeDescriptor, ObjArrayOop *bootstrapArgs) {
        const auto &methodClass = frame.klass;
        const auto &constantPool = methodClass.constantPool;
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;

        const auto runtimeMethodHandleClass = classLoader->getInstanceClass("Rex");
        const auto createCallSite = runtimeMethodHandleClass->getMethod("createCallSite", "(Ljava/lang/invoke/MethodHandle;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;", true);

        const auto [callSiteSlot, _] = frame.runMethodManual(*createCallSite, { 
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

    void invokeDynmic_(Frame &frame, u2 invokeDynamicIdx) {
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

        const auto bootstrapMethodHandle = createMethodHandle_(frame, methodHandleInfo);
        if (frame.markThrow) {
            return;
        }
        const auto bootstrapArguments = createBootstrapArgs(frame, bootstrapMethodAttr->bootstrapArguments);
        if (frame.markThrow) {
            return;
        }
        const auto callSiteObj = createCallSite(frame, bootstrapMethodHandle, invokeName, invokeDescriptor, bootstrapArguments);
        if (frame.markThrow) {
            return;
        }

        frame.pushRef(callSiteObj);
    }

}
