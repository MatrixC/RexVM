#include "exception_helper.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "string_pool.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    void throwAssignException(Frame &frame, cview className, cview message) {
        const auto instanceClass = frame.mem.getInstanceClass(className);
        instanceClass->clinit(frame);
        if (frame.markThrow) {
            return;
        }
        const auto throwable = frame.mem.newInstance(instanceClass);
        const auto hasMessage = !message.empty();
        const auto initMethod = 
            hasMessage ?
                instanceClass->getMethod("<init>" "(Ljava/lang/String;)V", false) :
                instanceClass->getMethod("<init>" "()V", false);
        std::vector<Slot> initParams;
        initParams.reserve(2);
        initParams.emplace_back(throwable);
        if (hasMessage) {
            initParams.emplace_back(frame.mem.getInternString(message));
        }
        frame.runMethodManual(*initMethod, initParams);
        if (frame.markThrow) {
            return;
        }
        frame.throwException(throwable);
    }

    void throwNullPointException(Frame &frame) {
        throwAssignException(frame, "java/lang/NullPointerException", {});
    }

    void throwArithmeticExceptionDivByZero(Frame &frame) {
        throwAssignException(frame, "java/lang/ArithmeticException", "/ by zero");
    }

    void throwReflectiveOperationException(Frame &frame, cview className, cview methodName, cview descriptor) {
        const auto message = cformat("java.lang.NoSuchMethodException: no such method: {}.{}#{}", getJavaClassName(className), methodName, descriptor);
        throwAssignException(frame, "java/lang/NoSuchMethodException", message);
    }

    void throwFileNotFoundException(Frame &frame, cview message) {
        throwAssignException(frame, "java/io/FileNotFoundException", message);
    }

    void throwIOException(Frame &frame, cview message) {
        throwAssignException(frame, "java/io/IOException", message);
    }

    void throwClassNotFoundException(Frame &frame, cview className) {
        throwAssignException(frame, "java/lang/ClassNotFoundException", className);
    }

    void throwClassCastException(Frame &frame, cview className1, cview className2) {
        const auto message = cformat("{} cannot be cast to {}", getJavaClassName(className1), getJavaClassName(className2));
        throwAssignException(frame, "java/lang/ClassCastException", message);
    }

    void throwRuntimeException(Frame &frame, cview message) {
        throwAssignException(frame, "java/lang/RuntimeException", message);
    }

    void throwIllegalThreadStateException(Frame &frame) {
        throwAssignException(frame, "java/lang/IllegalThreadStateException", {});
    }
}