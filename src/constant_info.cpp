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


    cstring getConstantString(ConstantInfo *info) {
        return static_cast<ConstantUTF8Info *>(info)->str;
    }

    std::tuple<const u1 *, u2> getConstantStringBytes(ConstantInfo *info) {
        auto utf8info = static_cast<ConstantUTF8Info *>(info);
        return std::make_tuple(utf8info->bytes.get(), utf8info->length);
    }

    cstring
    getConstantStringFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        return getConstantString(pool.at(index).get());
    }

    std::tuple<const u1 *, u2>
    getConstantStringBytesFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        return getConstantStringBytes(pool.at(index).get());
    }

    cstring getConstantStringFromPoolByIndexInfo(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        return getConstantStringFromPool(
            pool,
            static_cast<Constant1IndexInfo *>(pool[index].get())->index
        );
    }

    std::tuple<cstring, cstring> getConstantStringFromPoolByNameAndType(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        const auto nameAndTypeInfo = static_cast<ConstantNameAndTypeInfo *>(pool[index].get());
        return std::make_tuple(
            getConstantStringFromPool(pool, nameAndTypeInfo->nameIndex),
            getConstantStringFromPool(pool, nameAndTypeInfo->descriptorIndex)
        );
    }

    std::tuple<cstring, cstring, cstring> getConstantStringFromPoolByClassNameType(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        const auto classNameAndTypeInfo = static_cast<ConstantClassNameTypeIndexInfo *>(pool[index].get());
        const auto [name, type] = getConstantStringFromPoolByNameAndType(pool, classNameAndTypeInfo->nameAndTypeIndex);
        return std::make_tuple(
            getConstantStringFromPoolByIndexInfo(pool, classNameAndTypeInfo->classIndex),
            name,
            type
        );
    }
}