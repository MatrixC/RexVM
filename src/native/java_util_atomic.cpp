#include "java_util_atomic.hpp"

namespace RexVM::Native {

    void vmSupportsCS8(Frame &frame) {
        frame.returnBoolean(false);
    }

}