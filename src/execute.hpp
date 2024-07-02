#ifndef EXECUTE_HPP
#define EXECUTE_HPP
#include "config.hpp"
#include <vector>
#include <memory>

namespace RexVM {

    struct VM;
    struct Frame;
    struct VMThread;
    struct Method;

    void executeFrame(Frame &frame, [[maybe_unused]] const cstring& methodName);
    void createFrameAndRunMethod(VMThread &thread, Method &method_, std::vector<Slot> params, Frame *previous);
    void createFrameAndRunMethodNoPassParams(VMThread &thread, Method &method_, Frame *previous);
    void runStaticMethodOnMainThread(VM &vm, Method &method, std::vector<Slot> params);

}

#endif