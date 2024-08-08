#ifndef EXCEPTION_HELPER_HPP
#define EXCEPTION_HELPER_HPP
#include "config.hpp"

namespace RexVM {

    struct Frame;

    void throwAssignException(Frame &frame, const cstring &className, const cstring &message);

    void throwArithmeticExceptionDivByZero(Frame &frame);

    void throwNullPointException(Frame &frame);

    void throwReflectiveOperationException(Frame &frame, const cstring &className, const cstring &methodName, const cstring &descriptor);

    void throwFileNotFoundException(Frame &frame, const cstring &message);

    void throwIOException(Frame &frame, const cstring &message);

    void throwClassNotFoundException(Frame &frame, const cstring &className);

    void throwClassCastException(Frame &frame, const cstring &className1, const cstring &className2);
}

#endif
