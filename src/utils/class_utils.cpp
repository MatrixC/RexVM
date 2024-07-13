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

    cstring getDescriptorByClass(Class *klass) {
        if (klass->type == ClassTypeEnum::PrimitiveClass) {
            return PRIMITIVE_TYPE_MAP.at(klass->name);
        } else if (klass->type == ClassTypeEnum::InstanceClass) {
            return getDescriptorClassName(klass->name);
        } else {
            return klass->name;
        }
    }

    cstring getJVMClassName(const cstring &javaClassName) {
        return replace(javaClassName, ".", "/");
    }

    cstring getJavaClassName(const cstring &jvmClassName) {
        return replace(jvmClassName, "/", ".");
    }

}