#ifndef NATIVE_JAVA_SECURITY_ACCESS_CONTROLLER
#define NATIVE_JAVA_SECURITY_ACCESS_CONTROLLER
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void getStackAccessControlContext(Frame &frame);

}

#endif