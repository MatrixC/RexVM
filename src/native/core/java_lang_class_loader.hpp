#ifndef NATIVE_CORE_JAVA_LANG_CLASS_LOADER_HPP
#define NATIVE_CORE_JAVA_LANG_CLASS_LOADER_HPP
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

    //native void resolveClass0(Class<?> c);
    void resolveClass0(Frame &frame) {
        //same sa ensureClassInitialized
        const auto mirrorClassOop = static_cast<MirrorOop *>(frame.getLocalRef(1));
        const auto mirrorClass = mirrorClassOop->mirrorClass;
        if (mirrorClass->type == ClassTypeEnum::InstanceClass) {
            (static_cast<InstanceClass *>(mirrorClass))->clinit(frame);
        }
    }

    //native Class<?> findBootstrapClass(String name);
    void findBootstrapClass(Frame &frame) {
        const auto classNameOop = static_cast<InstanceOop *>(frame.getLocalRef(1));
        if (classNameOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = StringPool::getJavaString(classNameOop);
        const auto mirrorOop = frame.vm.bootstrapClassLoader->getClass(className)->getMirrorOop();
        frame.returnRef(mirrorOop);
    }

    //native final Class<?> findLoadedClass0(String name);
    void findLoadedClass0(Frame &frame) {
        const auto classNameOop = static_cast<InstanceOop *>(frame.getLocalRef(1));
        if (classNameOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = StringPool::getJavaString(classNameOop);
        const auto mirrorOop = frame.getCurrentClassLoader()->getClass(className)->getMirrorOop();
        frame.returnRef(mirrorOop);
    }







}


#endif