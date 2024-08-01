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
        const auto hasMessage = !message.empty();
        const auto initMethod = instanceClass->getMethod("<init>", hasMessage ? "(Ljava/lang/String;)V" : "()V", false);
        std::vector<Slot> initParams;
        initParams.reserve(2);
        initParams.emplace_back(throwable);
        if (hasMessage) {
            initParams.emplace_back(frame.vm.stringPool->getInternString(message));
        }
        frame.runMethodManual(*initMethod, initParams);
        frame.throwException(throwable);
    }

    void throwNullPointException(Frame &frame) {
        throwAssignException(frame, "java/lang/NullPointerException", {});
    }

    void throwArithmeticExceptionDivByZero(Frame &frame) {
        throwAssignException(frame, "java/lang/ArithmeticException", "/ by zero");
    }

    void throwReflectiveOperationException(Frame &frame, const cstring &className, const cstring &methodName, const cstring &descriptor) {
        const auto message = "java.lang.NoSuchMethodException: no such method: " + getJavaClassName(className) + "." + methodName + "#" + descriptor;
        throwAssignException(frame, "java/lang/NoSuchMethodException", message);
    }

    void throwFileNotFoundException(Frame &frame, const cstring &message) {
        throwAssignException(frame, "java/io/FileNotFoundException", message);
    }

    void throwIOException(Frame &frame, const cstring &message) {
        throwAssignException(frame, "java/io/IOException", message);
    }

    void throwClassNotFoundException(Frame &frame, const cstring &className) {
        throwAssignException(frame, "java/lang/ClassNotFoundException", className);
    }
}