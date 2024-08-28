#ifndef BASIC_TYPE_HPP
#define BASIC_TYPE_HPP

#include <unordered_map>
#include <array>
#include "config.hpp"
#include "exception.hpp"
#include "composite_ptr.hpp"

namespace RexVM {

    extern const std::unordered_map<cstring, cstring> PRIMITIVE_TYPE_MAP;
    extern const std::array<BasicType, 9> PRIMITIVE_TYPE_ARRAY;

    cstring getPrimitiveClassNameByDescriptor(cchar descriptor);
    cchar getDescriptorByPrimitiveClassName(const cstring &className);
    BasicType getBasicTypeByDescriptor(cchar descriptor);
    BasicType getBasicTypeByTypeArrayClassName(const cstring &className);

    extern cstring getTypeArrayClassNameByBasicType(BasicType type);
    cstring getPrimitiveClassNameByBasicType(BasicType type);

    bool isBasicType(cchar type);
    bool isWideBasicType(BasicType type);
    SlotTypeEnum getSlotTypeByPrimitiveClassName(const cstring &className);
    size_t getElementSizeByBasicType(BasicType type);

}

#endif