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
#include "../../string_pool.hpp"
#include "../../utils/string_utils.hpp"

namespace RexVM::Native::Sun::Misc {

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
    }

    void findSignal(Frame &frame) {
        frame.returnI4(-1);
    }

    void handle0(Frame &frame) {
        frame.returnI8(0);
    }

    void newInstance0(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
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
                const auto paramType = constructMethod->paramType[i];
                const auto val = paramArray->data[i];
                const auto paramClass = classLoader.getClass(paramType);
                if (paramClass->type == ClassTypeEnum::PRIMITIVE_CLASS) {
                    const auto primitiveClass = CAST_PRIMITIVE_CLASS(paramClass);
                    const auto slotVal = primitiveClass->getValueFromBoxingOop(CAST_INSTANCE_OOP(val));
                    constructorParams.emplace_back(slotVal);
                    if (primitiveClass->isWideType()) {
                        constructorParams.emplace_back(ZERO_SLOT);
                    }
                } else {
                    constructorParams.emplace_back(val);
                }
            }
        }

        frame.runMethodManual(*constructMethod, constructorParams);
        frame.returnRef(instance);
    }


    //native ByteBuffer createLong(String name, int variability, int units, long value);
    void createLong(Frame &frame) {
        //java.nio.ByteBuffer#allocateDirect
        const auto byteBufferClass = frame.getCurrentClassLoader()->getInstanceClass("java/nio/ByteBuffer");
        const auto allocateDirectMethod = byteBufferClass->getMethod("allocateDirect", "(I)Ljava/nio/ByteBuffer;", true);
        const auto [byteBufferOopSlot, slotType] = frame.runMethodManual(*allocateDirectMethod, { Slot(8) });
        frame.returnRef(byteBufferOopSlot.refVal);
    }

    //private static native int init();
    void undInit(Frame &frame) {
        frame.returnI4(0);
    }

    //static native byte[] getcwd();
    void undGetCwd(Frame &frame) {
        // std::array<char, 512> buffer;
        // getcwd(buffer.data(), sizeof(buffer));
        const auto buteArrayOop = frame.vm.oopManager->newByteArrayOop(0);
        frame.returnRef(buteArrayOop);
    }



}

#endif