#ifndef EXECUTE_HPP
#define EXECUTE_HPP
#include "config.hpp"
#include <vector>
#include <memory>

namespace RexVM {

    struct VM;
    struct Frame;
    struct Thread;
    struct Method;

    void executeFrame(Frame &frame, cstring methodName);

    void createFrameAndRunMethod(Thread &thread, Method &method_, std::vector<Slot> params, Frame *previous);

    void runStaticMethodOnMainThread(VM &vm, Method &method_, std::vector<Slot> params);
    Thread *runStaticMethodOnNewThread(VM &vm, Method &method_, std::vector<Slot> params);

}

#endif