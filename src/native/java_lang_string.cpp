#include "java_lang_string.hpp"
#include "../oop.hpp"
#include "../constant_pool.hpp"
#include "../vm.hpp"

namespace RexVM::Native {

    void intern(Frame &frame) {
        const auto self = frame.getThis();
        const auto str = StringPool::getJavaString(static_cast<InstanceOop *>(self));
        const auto strRef = frame.vm.stringPool->getInternString(str);
        frame.returnRef(strRef);
    }

}