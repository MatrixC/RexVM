#ifndef INTERPRETER_HELPER_HPP
#define INTERPRETER_HELPER_HPP

namespace RexVM {

    struct Frame;

    void throwArithmeticExceptionDivByZero(Frame &frame);
}

#endif
