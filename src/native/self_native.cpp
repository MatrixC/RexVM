#include "self_native.hpp"
#include "../oop.hpp"
#include "../vm.hpp"
#include "../constant_pool.hpp"

namespace RexVM::Native {

    void selfPrintln(Frame &frame) {
        auto str = frame.getLocalRef(0);
        const auto cstr = StringPool::getJavaString(CAST_INSTANCE_OOP(str));
        cprintln("{}", cstr);
    }

    void selfPrintlnInt(Frame &frame) {
        auto val = frame.getLocalI4(0);
        cprintln("{}", val);
    }
}