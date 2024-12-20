#ifndef NATIVE_CORE_JAVA_LANG_CLASS_LOADER_HPP
#define NATIVE_CORE_JAVA_LANG_CLASS_LOADER_HPP
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../string_pool.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/class_utils.hpp"

namespace RexVM::Native::Core {

    //native void resolveClass0(Class<?> c);
    void resolveClass0(Frame &frame) {
        //same sa ensureClassInitialized
        const auto mirrorClassOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorClass = mirrorClassOop->getMirrorClass();
        if (mirrorClass->type == ClassTypeEnum::INSTANCE_CLASS) {
            (CAST_INSTANCE_CLASS(mirrorClass))->clinit(frame);
        }
    }

    //native Class<?> findBootstrapClass(String name);
    void findBootstrapClass(Frame &frame) {
        const auto classNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        if (classNameOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = VMStringHelper::getJavaString(classNameOop);
        const auto mirrorOop = frame.mem.getClass(className)->getMirror(&frame);
        frame.returnRef(mirrorOop);
    }

    //native final Class<?> findLoadedClass0(String name);
    void findLoadedClass0(Frame &frame) {
        const auto classNameOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        if (classNameOop == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = getJVMClassName(VMStringHelper::getJavaString(classNameOop));
        const auto mirrorOop = frame.mem.getClass(className)->getMirror(&frame);
        frame.returnRef(mirrorOop);
    }







}


#endif