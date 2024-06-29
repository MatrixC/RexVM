#include "class_utils.hpp"
#include "string_utils.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../class_loader.hpp"

namespace RexVM {

    cstring getDescriptorClassName(const cstring &className) {
        return "L" + className + ";";
    }

    cstring getClassNameByFieldDescriptor(const cstring &descriptor) {
        const auto first = descriptor[0];
        if (first == '[') {
            return descriptor;
        }

        if (first == 'L') {
            return descriptor.substr(1, descriptor.size() - 2);
        }

        if (auto iter = PRIMITIVE_TYPE_REVERSE_MAP.find(descriptor); iter != PRIMITIVE_TYPE_REVERSE_MAP.end()) {
            return iter->second;
        }

        panic("error descriptor");
        return {};
    }
}