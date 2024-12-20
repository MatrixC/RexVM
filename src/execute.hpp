#ifndef EXECUTE_HPP
#define EXECUTE_HPP
#include "basic.hpp"
#include <vector>

#define EXCLUDE_EXECUTE_METHOD(mref, cname, mname, desc) \
    if (mref.getName() == mname && mref.getDescriptor() == desc && mref.klass.getClassName() == cname) { return; }

#define EXCLUDE_EXECUTE_METHODS(mref) \
EXCLUDE_EXECUTE_METHOD(mref, JAVA_LANG_SYSTEM_NAME, "loadLibrary", "(Ljava/lang/String;)V")

#ifdef DEBUG
#define PRINT_EXECUTE_LOG(o, f) \
    if (o) { cprintln("{}{}#{} {}", cstring(f.level * 2, ' '), f.klass.toView(), f.method.toView(), f.method.isNative() ? "[Native]" : ""); }
#else
#define PRINT_EXECUTE_LOG(o, f)
#endif

namespace RexVM {

    struct VM;
    struct Frame;
    struct VMThread;
    struct Method;

    extern bool printExecuteLog;

    void executeFrame(Frame &frame, [[maybe_unused]] cview methodName);
    void createFrameAndRunMethod(VMThread &thread, Method &method, Frame *previous, std::vector<Slot> params);
    void createFrameAndRunMethodNoPassParams(VMThread &thread, Method &method, Frame *previous, size_t paramSlotSize);

}

#endif