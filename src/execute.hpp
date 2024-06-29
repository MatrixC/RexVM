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

    void executeFrame(Frame &frame, [[maybe_unused]] const cstring& methodName);
    void createFrameAndRunMethod(Thread &thread, Method &method_, std::vector<Slot> params, Frame *previous);
    void createFrameAndRunMethodNoPassParams(Thread &thread, Method &method_, Frame *previous);
    void runStaticMethodOnThread(VM &vm, Method &method, std::vector<Slot> params, const cstring &name);
    void runStaticMethodOnNewThread(VM &vm, Method &method, std::vector<Slot> params);

}

#endif