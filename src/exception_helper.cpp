#include "exception_helper.hpp"
#include "frame.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "vm.hpp"
#include "memory.hpp"
#include "string_pool.hpp"
#include "key_slot_id.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    void throwAssignException(Frame &frame, const cstring &className, const cstring &message) {
        const auto instanceClass = frame.getCurrentClassLoader()->getInstanceClass(className);
        instanceClass->clinit(frame);
        const auto throwable = frame.vm.oopManager->newInstance(instanceClass);
        if (message != EMPTY_STRING) {
            const auto exceptionMessage = frame.vm.stringPool->getInternString("/ by zero");
            throwable->setFieldValue(throwableClassDetailMessageFieldSlotId, Slot(exceptionMessage));
        }
        frame.throwException(throwable);
    }

    void throwArithmeticExceptionDivByZero(Frame &frame) {
        throwAssignException(frame, "java/lang/ArithmeticException", "/ by zero");
    }

    void throwReflectiveOperationException(Frame &frame, const cstring &className, const cstring &methodName, const cstring &descriptor) {
        const auto message = "java.lang.NoSuchMethodException: no such method: " + getJavaClassName(className) + "." + methodName + descriptor;
        throwAssignException(frame, "java/lang/NoSuchMethodException", message);
    }

    void throwFileNotFoundException(Frame &frame, const cstring &message) {
        throwAssignException(frame, "java/io/FileNotFoundException", message);
    }
}