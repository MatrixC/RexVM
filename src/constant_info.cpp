#include "constant_info.hpp"

namespace RexVM {
    std::unordered_map<ConstantTagEnum, cstring> CONST_NAME_MAP = {
            {ConstantTagEnum::CONSTANT_EMPTY,              "EMPTY"},
            {ConstantTagEnum::CONSTANT_Class,              "Class"},
            {ConstantTagEnum::CONSTANT_FieldRef,           "FieldRef"},
            {ConstantTagEnum::CONSTANT_MethodRef,          "MethodRef"},
            {ConstantTagEnum::CONSTANT_InterfaceMethodRef, "InterfaceMethodRef"},
            {ConstantTagEnum::CONSTANT_String,             "String"},
            {ConstantTagEnum::CONSTANT_Integer,            "Integer"},
            {ConstantTagEnum::CONSTANT_Float,              "Float"},
            {ConstantTagEnum::CONSTANT_Long,               "Long"},
            {ConstantTagEnum::CONSTANT_Double,             "Double"},
            {ConstantTagEnum::CONSTANT_NameAndType,        "NameAndType"},
            {ConstantTagEnum::CONSTANT_Utf8,               "Utf8"},
            {ConstantTagEnum::CONSTANT_MethodHandle,       "MethodHandle"},
            {ConstantTagEnum::CONSTANT_MethodType,         "MethodType"},
            {ConstantTagEnum::CONSTANT_InvokeDynamic,      "InvokeDynamic"},
    };
}