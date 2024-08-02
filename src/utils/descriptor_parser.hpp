#ifndef DESCRIPTOR_PARSER_HPP
#define DESCRIPTOR_PARSER_HPP
#include "../config.hpp"
#include "../basic_type.hpp"
#include "format.hpp"
#include <memory>
#include <vector>
#include <tuple>

namespace RexVM {

    std::tuple<size_t, u1, u1> getSingleDescriptor(const cview descriptor);

    std::tuple<std::vector<cstring>, cstring> parseMethodDescriptor(cview str);

    std::vector<cstring> parseDescriptor(cview str);
    
    std::vector<cstring> parseDescriptor(const cview str, size_t start, size_t end);

    size_t getMethodParamSlotSizeFromDescriptor(const cview descriptor, bool isStatic);
}

#endif