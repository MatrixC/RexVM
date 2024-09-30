#include "basic_type.hpp"
#include "exception.hpp"

namespace RexVM {
    constexpr cview PRIMITIVE_VOID_CLASS_NAME = "void";
    constexpr cview PRIMITIVE_BOOLEAN_CLASS_NAME = "boolean";
    constexpr cview PRIMITIVE_BYTE_CLASS_NAME = "byte";
    constexpr cview PRIMITIVE_SHORT_CLASS_NAME = "short";
    constexpr cview PRIMITIVE_INT_CLASS_NAME = "int";
    constexpr cview PRIMITIVE_LONG_CLASS_NAME = "long";
    constexpr cview PRIMITIVE_CHAR_CLASS_NAME = "char";
    constexpr cview PRIMITIVE_FLOAT_CLASS_NAME = "float";
    constexpr cview PRIMITIVE_DOUBLE_CLASS_NAME = "double";

    constexpr cview PRIMITIVE_BOOLEAN_ARRAY_CLASS_NAME = "[Z";
    constexpr cview PRIMITIVE_BYTE_ARRAY_CLASS_NAME = "[B";
    constexpr cview PRIMITIVE_SHORT_ARRAY_CLASS_NAME = "[S";
    constexpr cview PRIMITIVE_INT_ARRAY_CLASS_NAME = "[I";
    constexpr cview PRIMITIVE_LONG_ARRAY_CLASS_NAME = "[J";
    constexpr cview PRIMITIVE_CHAR_ARRAY_CLASS_NAME = "[C";
    constexpr cview PRIMITIVE_FLOAT_ARRAY_CLASS_NAME = "[F";
    constexpr cview PRIMITIVE_DOUBLE_ARRAY_CLASS_NAME = "[D";
    

    cchar getDescriptorByPrimitiveClassName(cview className) {
        if (className == "void") {
            return 'V';
        } else if (className == "boolean") {
            return 'Z';
        } else if (className == "byte") {
            return 'B';
        } else if (className == "short") {
            return 'S';
        } else if (className == "int") {
            return 'I';
        } else if (className == "long") {
            return 'J';
        } else if (className == "char") {
            return 'C';
        } else if (className == "float") {
            return 'F';
        } else if (className == "double") {
            return 'D';
        }
        panic("error class name");
        return 'V';
    }

    cview getPrimitiveClassNameByDescriptor(cchar descriptor) {
        switch (descriptor) {
            case 'V':
                return PRIMITIVE_VOID_CLASS_NAME;
            case 'Z':
                return PRIMITIVE_BOOLEAN_CLASS_NAME;
            case 'B':
                return PRIMITIVE_BYTE_CLASS_NAME;
            case 'S':
                return PRIMITIVE_SHORT_CLASS_NAME;
            case 'I':
                return PRIMITIVE_INT_CLASS_NAME;
            case 'J':
                return PRIMITIVE_LONG_CLASS_NAME;
            case 'C':
                return PRIMITIVE_CHAR_CLASS_NAME;
            case 'F':
                return PRIMITIVE_FLOAT_CLASS_NAME;
            case 'D':
                return PRIMITIVE_DOUBLE_CLASS_NAME;
            default:
                panic("error descriptor descriptor");
        }
        return PRIMITIVE_VOID_CLASS_NAME;
    }

    cview getPrimitiveClassNameByBasicType(BasicType type) {
        switch (type) {
            case BasicType::T_VOID:
                return PRIMITIVE_VOID_CLASS_NAME;
            case BasicType::T_BOOLEAN:
                return PRIMITIVE_BOOLEAN_CLASS_NAME;
            case BasicType::T_BYTE:
                return PRIMITIVE_BYTE_CLASS_NAME;
            case BasicType::T_SHORT:
                return PRIMITIVE_SHORT_CLASS_NAME;
            case BasicType::T_INT:
                return PRIMITIVE_INT_CLASS_NAME;
            case BasicType::T_LONG:
                return PRIMITIVE_LONG_CLASS_NAME;
            case BasicType::T_CHAR:
                return PRIMITIVE_CHAR_CLASS_NAME;
            case BasicType::T_FLOAT:
                return PRIMITIVE_FLOAT_CLASS_NAME;
            case BasicType::T_DOUBLE:
                return PRIMITIVE_DOUBLE_CLASS_NAME;
            
            default:
                panic("type error");
                break;
        }
        return PRIMITIVE_VOID_CLASS_NAME;
    }

    BasicType getBasicTypeByDescriptor(cchar descriptor) {
        switch (descriptor) {
            case 'V':
                return BasicType::T_VOID;
            case 'Z':
                return BasicType::T_BOOLEAN;
            case 'B':
                return BasicType::T_BYTE;
            case 'S':
                return BasicType::T_SHORT;
            case 'I':
                return BasicType::T_INT;
            case 'J':
                return BasicType::T_LONG;
            case 'C':
                return BasicType::T_CHAR;
            case 'F':
                return BasicType::T_FLOAT;
            case 'D':
                return BasicType::T_DOUBLE;
            default:
                panic("error descriptor descriptor");
        }
        return BasicType::T_ILLEGAL;
    }

    BasicType getBasicTypeByTypeArrayClassName(cview className) {
        if (className == "[Z") {
            return BasicType::T_BOOLEAN;
        } else if (className == "[B") {
            return BasicType::T_BYTE;
        } else if (className == "[S") {
            return BasicType::T_SHORT;
        } else if (className == "[I") {
            return BasicType::T_INT;
        } else if (className == "[C") {
            return BasicType::T_CHAR;
        } else if (className == "[J") {
            return BasicType::T_LONG;
        } else if (className == "[F") {
            return BasicType::T_FLOAT;
        } else if (className == "[D") {
            return BasicType::T_DOUBLE;
        }
        panic(cformat("getBasicTypeByTypeArrayClassName error: error className {}", className));
        return BasicType::T_VOID;
    }

    bool isBasicType(cchar type) {
        return type == 'Z' || type == 'B' || type == 'S' || type == 'I' ||
               type == 'J' || type == 'C' || type == 'F' || type == 'D' || type == 'V';
    }

    cview getTypeArrayClassNameByBasicType(BasicType type) {
        switch (type) {
            case BasicType::T_BOOLEAN:
                return PRIMITIVE_BOOLEAN_ARRAY_CLASS_NAME;
            case BasicType::T_CHAR:
                return PRIMITIVE_CHAR_ARRAY_CLASS_NAME;
            case BasicType::T_FLOAT:
                return PRIMITIVE_FLOAT_ARRAY_CLASS_NAME;
            case BasicType::T_DOUBLE:
                return PRIMITIVE_DOUBLE_ARRAY_CLASS_NAME;
            case BasicType::T_BYTE:
                return PRIMITIVE_BYTE_ARRAY_CLASS_NAME;
            case BasicType::T_SHORT:
                return PRIMITIVE_SHORT_ARRAY_CLASS_NAME;
            case BasicType::T_INT:
                return PRIMITIVE_INT_ARRAY_CLASS_NAME;
            case BasicType::T_LONG:
                return PRIMITIVE_LONG_ARRAY_CLASS_NAME;
            default:
                panic("type error");
                break;
        }
        return PRIMITIVE_BOOLEAN_ARRAY_CLASS_NAME;
    }


    bool isWideBasicType(BasicType type) {
        return type == BasicType::T_LONG || type == BasicType::T_DOUBLE;
    }

    const std::array<BasicType, 9> PRIMITIVE_TYPE_ARRAY = {
            BasicType::T_BOOLEAN,
            BasicType::T_CHAR,
            BasicType::T_FLOAT,
            BasicType::T_DOUBLE,
            BasicType::T_BYTE,
            BasicType::T_SHORT,
            BasicType::T_INT,
            BasicType::T_LONG,
            BasicType::T_VOID,
    };

    SlotTypeEnum getSlotTypeByPrimitiveClassName(cview className) {
        SlotTypeEnum slotType;
        if (className == "boolean" || className == "byte" || className == "short" || className == "int" ||
            className == "char") {
            slotType = SlotTypeEnum::I4;
        } else if (className == "long") {
            slotType = SlotTypeEnum::I8;
        } else if (className == "float") {
            slotType = SlotTypeEnum::F4;
        } else if (className == "double") {
            slotType = SlotTypeEnum::F8;
        } else if (className == "void") {
            slotType = SlotTypeEnum::NONE;
        } else {
            slotType = SlotTypeEnum::REF;
        }

        return slotType;
    }

    size_t getElementSizeByBasicType(BasicType type) {
        switch (type) {
            case BasicType::T_BOOLEAN:
            case BasicType::T_BYTE:
                return sizeof(u1);
            case BasicType::T_CHAR:
                return sizeof(cchar_16);
            case BasicType::T_FLOAT:
                return sizeof(f4);
            case BasicType::T_DOUBLE:
                return sizeof(f8);
            case BasicType::T_SHORT:
                return sizeof(i2);
            case BasicType::T_INT:
                return sizeof(i4);
            case BasicType::T_LONG:
                return sizeof(i8);
            default:
                panic("type error");
                break;
        }
        return {};
    }
}