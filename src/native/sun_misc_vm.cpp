#include "sun_misc_vm.hpp"
#include "../class_loader.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../vm.hpp"
#include "../memory.hpp"
#include "../utils/class_utils.hpp"
#include <unordered_map>
#include <csignal>

namespace RexVM::Native {

    void initialize(Frame &frame) {
        // auto systemClass = frame.classLoader.getInstanceClass("java/lang/System");
        // auto initMethod = systemClass->getMethod("initializeSystemClass", "()V", true);
        // frame.runMethod(*initMethod);
    }


  
    void newInstance0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto constructor = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto paramArray = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(1));

        const auto srcClassOop = CAST_MIRROR_OOP(constructor->getFieldValue("clazz",
                                                                                     "Ljava/lang/Class;").refVal);
        const auto slotId = constructor->getFieldValue("slot", "I").i4Val;
        const auto srcClass = dynamic_cast<InstanceClass *>(srcClassOop->mirrorClass);
        const auto &constructMethod = srcClass->methods.at(slotId);

        const auto instance = oopManager->newInstance(srcClass);
        std::vector<Slot> constructorParams;
        constructorParams.emplace_back(instance);
        if (paramArray != nullptr) {
            constructorParams.reserve(paramArray->dataLength);
            for (size_t i = 0; i < paramArray->dataLength; ++i) {
                constructorParams.emplace_back(paramArray->data[i]);
            }
        }
        frame.runMethodManual(*constructMethod, constructorParams);

        frame.returnRef(instance);
    }


    void putLong(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto value = frame.getLocalI8(3);
        const auto address = std::bit_cast<i8 *>(addressLong);
        *address = value;
    }

    void getByte(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<i1 *>(addressLong);
        frame.returnI4(*address);
    }



    //from signum-generic.h
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

    void findSignal(Frame &frame) {
//        const auto sigName = getStringNativeValue(static_cast<Oop *>(frame.getLocalRef(0)));
//        if (auto iter = SIGNAL_NUMBER_MAP.find(sigName); iter != SIGNAL_NUMBER_MAP.end()) {
//            frame.returnI4(iter->second);
//            return;
//        }
        frame.returnI4(-1);
    }

    void handle0(Frame &frame) {
        frame.returnI8(0);
    }

}