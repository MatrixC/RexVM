#ifndef EXCEPTION_HELPER_HPP
#define EXCEPTION_HELPER_HPP
#include "config.hpp"

namespace RexVM {

    struct Frame;

    void throwAssignException(Frame &frame, cview className, cview message);

    void throwArithmeticExceptionDivByZero(Frame &frame);

    void throwNullPointException(Frame &frame);

    void throwReflectiveOperationException(Frame &frame, cview className, cview methodName, cview descriptor);

    void throwFileNotFoundException(Frame &frame, cview message);

    void throwIOException(Frame &frame, cview message);

    void throwClassNotFoundException(Frame &frame, cview className);

    void throwClassCastException(Frame &frame, cview className1, cview className2);

    void throwRuntimeException(Frame &frame, cview message);

    void throwIllegalThreadStateException(Frame &frame);
}

#endif
