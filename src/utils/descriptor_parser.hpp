#ifndef DESCRIPTOR_PARSER_HPP
#define DESCRIPTOR_PARSER_HPP
#include "../config.hpp"
#include "../basic_type.hpp"
#include "format.hpp"
#include <memory>
#include <vector>
#include <tuple>

namespace RexVM {

    struct DescriptorFieldType {
        bool isBasicType{true};
        BasicType basicType;
        cstring className;

        cstring format();
    };

    struct MethodDescriptorResult {
        std::vector<cstring> parameterFieldType;
        cstring returnType;
    };

    cview descriptorNextFieldType(const cview str);
    std::vector<cstring> parseDescriptor(const cview str);
    std::tuple<std::vector<cstring>, cstring> parseMethodDescriptor(const cview str);

}

#endif