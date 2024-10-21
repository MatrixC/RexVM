#ifndef BASIC_TYPE_HPP
#define BASIC_TYPE_HPP

#include <array>
#include "config.hpp"

namespace RexVM {
    extern const std::array<BasicType, 9> PRIMITIVE_TYPE_ARRAY;

    cview getPrimitiveClassNameByDescriptor(cchar descriptor);
    cview getPrimitiveClassNameByBasicType(BasicType type);

    cchar getDescriptorByPrimitiveClassName(cview lassName);
    BasicType getBasicTypeByDescriptor(cchar descriptor);
    BasicType getBasicTypeByTypeArrayClassName(cview className);

    cview getTypeArrayClassNameByBasicType(BasicType type);
    
    bool isBasicType(cchar type);
    bool isWideBasicType(BasicType type);
    SlotTypeEnum getSlotTypeByPrimitiveClassName(cview className);
    size_t getElementSizeByBasicType(BasicType type);

}

#endif