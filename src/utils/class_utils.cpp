#include "class_utils.hpp"
#include "string_utils.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../class_loader.hpp"

namespace RexVM {

    cstring getDescriptorClassName(cview className) {
        return "L" + cstring(className) + ";";
    }

    cstring getClassNameByFieldDescriptor(cview descriptor) {
        const auto first = descriptor[0];
        if (first == '[') {
            return cstring(descriptor);
        }

        if (first == 'L') {
            return cstring(descriptor.substr(1, descriptor.size() - 2));
        }

        return getPrimitiveClassNameByDescriptor(first);
    }

    cstring getDescriptorByClass(Class *klass) {
        if (klass->getType() == ClassTypeEnum::PRIMITIVE_CLASS) {
            const auto descriptor = getDescriptorByPrimitiveClassName(klass->name);
            return cstring{CAST_CHAR(descriptor)};
        } else if (klass->getType() == ClassTypeEnum::INSTANCE_CLASS) {
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

    bool isWideClassName(const cstring &className) {
        return className == "long" || className == "double";
    }

}