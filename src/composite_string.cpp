#include "composite_string.hpp"
#include <cstring>

namespace RexVM {

    CompositeString::CompositeString(const cchar *str) {
        if (str == nullptr) {
            return;
        }
        const auto size = strlen(str);
        if (size > COM_DATA_SHIFT_MASK) {
            panic("not support length");
        }
        reset(str, CAST_U2(size));
    }

}