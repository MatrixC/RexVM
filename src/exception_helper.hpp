#ifndef EXCEPTION_HELPER_HPP
#define EXCEPTION_HELPER_HPP
#include "config.hpp"

namespace RexVM {

    struct Frame;

    void throwArithmeticExceptionDivByZero(Frame &frame);

    void throwReflectiveOperationException(Frame &frame, const cstring &className, const cstring &methodName, const cstring &descriptor);

    void throwFileNotFoundException(Frame &frame, const cstring &message);
}

#endif
