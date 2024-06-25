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

    void arrayBaseOffset(Frame &frame) {
        //TODO
        frame.returnI4(0);
    }

    void arrayIndexScale(Frame &frame) {
        //TODO
        frame.returnI4(1);
    }

    void addressSize(Frame &frame) {
        //TODO
        frame.returnI4(sizeof(ref));
    }

    void getCallerClass(Frame &frame) {
        const auto callerFrame = frame.previous->previous;
        frame.returnRef(callerFrame->klass.getMirrorOop());
    }

    void objectFieldOffset(Frame &frame) {
        const auto fieldMirror = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto value = fieldMirror->getFieldValue("slot", "I").i4Val;
        frame.returnI8(value);
    }

    //(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z 
    void compareAndSwapObject(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);
        //const auto expected = frame.getLocalRef(3);
        const auto newValue = frame.getLocalRef(4);

        obj->data[offset] = Slot(newValue);
        frame.returnBoolean(true);
    }

    void compareAndSwapInt(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);
        //const auto expected = frame.getLocalI4(3);
        const auto newValue = frame.getLocalI4(4);

        obj->data[offset] = Slot(newValue);
        frame.returnBoolean(true);
    }

    void compareAndSwapLong(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);
        //const auto expected = frame.getLocalI8(3);
        const auto newValue = frame.getLocalI8(4);

        obj->data[offset] = Slot(newValue);
        frame.returnBoolean(true);
    }

    void getObjectVolatile(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);

        frame.returnRef(obj->data[offset].refVal);
    }

    void putObjectVolatile(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);
        const auto newValue = frame.getLocal(3);

        obj->data[offset] = newValue;
    }

    void getIntVolatile(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);

        frame.returnI4(obj->data[offset].i4Val);
    }

    void putIntVolatile(Frame &frame) {
        const auto obj = static_cast<InstanceOop *>(frame.getLocalRef(1));
        const auto offset = frame.getLocalI8(2);
        const auto newValue = frame.getLocal(3);

        obj->data[offset] = newValue;
    }

    void getClassAccessFlags(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnI4(static_cast<i4>(mirrorClass->accessFlags));
    }

    void newInstance0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto constructor = static_cast<InstanceOop *>(frame.getLocalRef(0));
        const auto paramArray = static_cast<ObjArrayOop *>(frame.getLocalRef(1));

        const auto srcClassOop = static_cast<MirrorOop *>(constructor->getFieldValue("clazz",
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
        frame.runMethod(*constructMethod, constructorParams);

        frame.returnRef(instance);
    }

    void allocateMemory(Frame &frame) {
        const auto length = frame.getLocalI8(1);
        const auto ptr = std::malloc(length);
        frame.returnI8(std::bit_cast<i8>(ptr));
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

    void freeMemory(Frame &frame) {
        const auto addressLong = frame.getLocalI8(1);
        const auto address = std::bit_cast<void *>(addressLong);
        std::free(address);
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