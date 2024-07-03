#ifndef NATIVE_CORE_JAVA_LANG_CLASS_HPP
#define NATIVE_CORE_JAVA_LANG_CLASS_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../constant_pool.hpp"
#include "../../utils/string_utils.hpp"

namespace RexVM::Native::Core {

    void getName0(Frame &frame) {
        const auto instance = static_cast<MirrorOop *>(frame.getThisInstance());
        const auto mirrorClass = instance->mirrorClass;
        const auto className = mirrorClass->name;
        const auto javaClassName = replace(className, "/", ".");
        auto strOop = frame.vm.stringPool->getInternString(javaClassName);
        frame.returnRef(strOop);
    }

    void getEnclosingMethod0(Frame &frame) {
        //Class Instance
        const auto mirrorOop = static_cast<MirrorOop *>(frame.getThisInstance());
        const auto mirrorClass = mirrorOop->mirrorClass;
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = static_cast<InstanceClass *>(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto enclosingMethodAttr = mirrorInstanceClass->getEnclosingMethodAttr();
        if (enclosingMethodAttr == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, enclosingMethodAttr->classIndex);
        const auto objArrayOop = 
            frame.vm.oopManager->newObjArrayOop(
                frame.classLoader.getObjectArrayClass(JAVA_LANG_OBJECT_NAME), 
                3
            );
        objArrayOop->data[0] = frame.vm.bootstrapClassLoader->getClass(className)->getMirrorOop();

        const auto [methodName, methodDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, enclosingMethodAttr->methodIndex);
        objArrayOop->data[1] = frame.vm.stringPool->getInternString(methodName);
        objArrayOop->data[2] = frame.vm.stringPool->getInternString(methodDescriptor);

        frame.returnRef(objArrayOop);
    }

    void getDeclaringClass0(Frame &frame) {
        const auto mirrorOop = static_cast<MirrorOop *>(frame.getThisInstance());
        const auto mirrorClass = mirrorOop->mirrorClass;
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = static_cast<InstanceClass *>(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto innerClassesAttr = mirrorInstanceClass->getInnerClassesAttr();
        if (innerClassesAttr == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        for (const auto &item : innerClassesAttr->classes) {
            if (item->innerClassInfoIndex != 0) {
                const auto innerClassName = getConstantStringFromPoolByIndexInfo(constantPool, item->innerClassInfoIndex);
                if (innerClassName == mirrorInstanceClass->name) {
                    if (item->outerClassInfoIndex == 0) {
                        frame.returnRef(nullptr);
                        return;
                    } else {
                        const auto outerClassName = getConstantStringFromPoolByIndexInfo(constantPool, item->outerClassInfoIndex);
                        frame.returnRef(frame.vm.bootstrapClassLoader->getClass(outerClassName)->getMirrorOop());
                        return;
                    }
                } 
            }
        }
    }

}


#endif