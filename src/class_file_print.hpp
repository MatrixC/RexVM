#ifndef CLASS_FILE_PRINT_HPP
#define CLASS_FILE_PRINT_HPP

#include "utils/format.hpp"
#include "constant_info.hpp"
#include "attribute_info.hpp"
#include "class_file.hpp"


namespace RexVM {

    inline void printAccessFlag(u2 accessFlag) {

    }

    inline void printBase(ClassFile &cf) {

    }

    inline void printConstant(ClassFile &cf) {
        for (auto i = 0; i < cf.constantPoolCount; ++i) {
            const auto &constantPtr = cf.constantPool.at(i);
            if (constantPtr != nullptr) {
                cprintln("{:>2} = {}", i, constantPtr->toString());
            }
        }
    }

    inline void printMethods(ClassFile &cf) {
        cprintln("method count {}", cf.methodCount);
        for (const auto &methodInfo: cf.methods) {
            cprintln("{}, {}, {}, {}", methodInfo->accessFlags, methodInfo->nameIndex, methodInfo->descriptorIndex,
                    methodInfo->attributesCount);
        }
    }

    inline void printCF(ClassFile &cf) {
        printMethods(cf);
    }

}

#endif