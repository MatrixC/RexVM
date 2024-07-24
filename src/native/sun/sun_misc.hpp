#ifndef NATIVE_SUN_MISC_HPP
#define NATIVE_SUN_MISC_HPP
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

namespace RexVM::Native::SUN::MISC {

    const std::unordered_map<cstring, i4> SIGNAL_NUMBER_MAP{
            {"INT", 2},
            {"ILL", 4},
            {"ABRT", 6},
            {"FPE", 8},
            {"SEGV", 11},
            {"TERM", 15},
            {"HUP", 1},
            {"QUIT", 3},
            {"TRAP", 5},
            {"KILL", 9},
            {"PIPE", 13},
            {"ALRM", 14},
    };

    void getLookupCacheURLs(Frame &frame) {
        frame.returnRef(nullptr);
    }

    void initialize(Frame &frame) {
        // auto systemClass = frame.classLoader.getInstanceClass("java/lang/System");
        // auto initMethod = systemClass->getMethod("initializeSystemClass", "()V", true);
        // frame.runMethod(*initMethod);
    }

    void findSignal(Frame &frame) {
        frame.returnI4(-1);
    }

    void handle0(Frame &frame) {
        frame.returnI8(0);
    }

    void newInstance0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto constructor = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto paramArray = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(1));

        const auto srcClassOop = CAST_MIRROR_OOP(constructor->getFieldValue("clazz", "Ljava/lang/Class;").refVal);
        const auto slotId = constructor->getFieldValue("slot", "I").i4Val;
        const auto srcClass = CAST_INSTANCE_CLASS(srcClassOop->mirrorClass);
        const auto &constructMethod = srcClass->methods.at(slotId);

        const auto instance = oopManager->newInstance(srcClass);
        std::vector<Slot> constructorParams;
        constructorParams.emplace_back(instance);
        if (paramArray != nullptr) {
            for (size_t i = 0; i < paramArray->dataLength; ++i) {
                constructorParams.emplace_back(paramArray->data[i]);
            }
        }
        frame.runMethodManual(*constructMethod, constructorParams);
        frame.returnRef(instance);
    }



}

#endif