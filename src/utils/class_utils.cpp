#include "class_utils.hpp"
#include "string_utils.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../class_loader.hpp"

namespace RexVM {

    cview getClassNameByFieldDescriptor(cview descriptor) {
        const auto first = descriptor[0];
        if (first == '[') {
            //Array
            return descriptor;
        }

        if (first == 'L') {
            //InstanceClass
            return descriptor.substr(1, descriptor.size() - 2);
        }

        //Primitive
        return getPrimitiveClassNameByDescriptor(first);
    }

    cstring getJVMClassName(cview javaClassName) {
        cstring result;
        result.reserve(javaClassName.size());
        for (const auto &c : javaClassName) {
            result += c == '.' ? '/' : c;
        }
        return result;
    }

    cstring getJavaClassName(cview jvmClassName) {
        cstring result;
        result.reserve(jvmClassName.size());
        for (const auto &c : jvmClassName) {
            result += c == '/' ? '.' : c;
        }
        return result;
    }

    bool isWideClassName(cview className) {
        return className == "long" || className == "double";
    }

}