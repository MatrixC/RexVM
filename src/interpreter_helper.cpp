#include "interpreter_helper.hpp"
#include "frame.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "vm.hpp"
#include "memory.hpp"
#include "constant_pool.hpp"
#include "key_slot_id.hpp"

namespace RexVM {

    void throwArithmeticExceptionDivByZero(Frame &frame) {
        const auto instanceClass = frame.classLoader.getInstanceClass("java/lang/ArithmeticException");
        instanceClass->clinit(frame);
        const auto throwable = frame.vm.oopManager->newInstance(instanceClass);
        const auto exceptionMessage = frame.vm.stringPool->getInternString("/ by zero");
        throwable->setFieldValue(throwableClassDetailMessageFieldSlotId, Slot(exceptionMessage));
        frame.throwException(throwable);
    }
}