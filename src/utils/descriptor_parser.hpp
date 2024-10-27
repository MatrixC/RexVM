#ifndef DESCRIPTOR_PARSER_HPP
#define DESCRIPTOR_PARSER_HPP
#include "../basic.hpp"
#include "format.hpp"
#include <vector>
#include <tuple>

namespace RexVM {

    std::tuple<size_t, u1, u1> getSingleDescriptor(cview descriptor);

    std::tuple<std::vector<cstring>, cstring> parseMethodDescriptor(cview str);

    std::vector<cstring> parseDescriptor(cview str);
    
    std::vector<cstring> parseDescriptor(cview str, size_t start, size_t end);

    size_t getMethodParamSlotSizeFromDescriptor(cview descriptor, bool isStatic);
}

#endif