#include "constant_info.hpp"

namespace RexVM {
    emhash8::HashMap<ConstantTagEnum, cview> CONST_NAME_MAP = {
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


    cview getConstantString(ConstantInfo *info) {
        const auto utf8Info = CAST_CONSTANT_UTF_8_INFO(info);
        return {reinterpret_cast<char *>(utf8Info->bytes.get()), utf8Info->length};
    }

    rstring getConstantRString(ConstantInfo *info) {
        const auto utf8Info = CAST_CONSTANT_UTF_8_INFO(info);
        return {reinterpret_cast<char *>(utf8Info->bytes.get()), utf8Info->length};
    }

    std::tuple<const u1 *, u2> getConstantStringBytes(ConstantInfo *info) {
        const auto utf8info = CAST_CONSTANT_UTF_8_INFO(info);
        return std::make_tuple(utf8info->bytes.get(), utf8info->length);
    }

    cview
    getConstantStringFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        const auto info = pool[index].get();
        return getConstantString(info);
    }

    rstring
    getConstantRStringFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        const auto info = pool[index].get();
        return getConstantRString(info);
    }

    std::tuple<const u1 *, u2>
    getConstantStringBytesFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        return getConstantStringBytes(pool[index].get());
    }

    cview getConstantStringFromPoolByIndexInfo(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        return getConstantStringFromPool(
            pool,
            CAST_CONSTANT_1_INDEX_INFO(pool[index].get())->index
        );
    }

    rstring getConstantRStringFromPoolByIndexInfo(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        return getConstantRStringFromPool(
            pool,
            CAST_CONSTANT_1_INDEX_INFO(pool[index].get())->index
        );
    }

    std::tuple<cview, cview> getConstantStringFromPoolByNameAndType(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        const auto nameAndTypeInfo = CAST_CONSTANT_NAME_AND_TYPE_INFO(pool[index].get());
        return std::make_tuple(
            getConstantStringFromPool(pool, nameAndTypeInfo->nameIndex),
            getConstantStringFromPool(pool, nameAndTypeInfo->descriptorIndex)
        );
    }

    std::tuple<cview, cview, cview> getConstantStringFromPoolByClassNameType(
        const std::vector<std::unique_ptr<ConstantInfo>> &pool,
        const size_t index
    ) {
        const auto classNameAndTypeInfo = CAST_CONSTANT_CLASS_NAME_TYPE_INDEX_INFO(pool[index].get());
        const auto [name, type] = getConstantStringFromPoolByNameAndType(pool, classNameAndTypeInfo->nameAndTypeIndex);
        return std::make_tuple(
            getConstantStringFromPoolByIndexInfo(pool, classNameAndTypeInfo->classIndex),
            name,
            type
        );
    }
}