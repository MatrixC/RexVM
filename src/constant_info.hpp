#ifndef CONSTANT_INFO_HPP
#define CONSTANT_INFO_HPP

#include <bit>
#include <unordered_map>
#include <vector>
#include <tuple>
#include "config.hpp"
#include "utils/binary.hpp"

namespace RexVM {

    enum class ConstantTagEnum : u1 {
        CONSTANT_EMPTY = 0,
        CONSTANT_Utf8 = 1,
        CONSTANT_Integer = 3,
        CONSTANT_Float = 4,
        CONSTANT_Long = 5,
        CONSTANT_Double = 6,
        CONSTANT_Class = 7,
        CONSTANT_String = 8,
        CONSTANT_FieldRef = 9,
        CONSTANT_MethodRef = 10,
        CONSTANT_InterfaceMethodRef = 11,
        CONSTANT_NameAndType = 12,
        CONSTANT_MethodHandle = 15,
        CONSTANT_MethodType = 16,
        CONSTANT_Dynamic = 17,
        CONSTANT_InvokeDynamic = 18,
        CONSTANT_Module = 19,
        CONSTANT_PACKAGE = 20,
    };

    extern std::unordered_map<ConstantTagEnum, cstring> CONST_NAME_MAP;

    struct ConstantInfo {
        u1 tag = 0;

        explicit ConstantInfo(std::istream &is) {
            read(tag, is);
        }

        virtual ~ConstantInfo() = default;

        virtual cstring toString() = 0;
    };

    struct Constant1IndexInfo : public ConstantInfo {
        u2 index{};

        explicit Constant1IndexInfo(std::istream &is) : ConstantInfo(is) {
            read(index, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), index);
        }
    };


    struct ConstantClassInfo : public Constant1IndexInfo {
        explicit ConstantClassInfo(std::istream &is) : Constant1IndexInfo(is) {
        }
    };

    struct ConstantStringInfo : public Constant1IndexInfo {
        explicit ConstantStringInfo(std::istream &is) : Constant1IndexInfo(is) {
        }
    };

    struct ConstantClassNameTypeIndexInfo : public ConstantInfo {
        u2 classIndex{};
        u2 nameAndTypeIndex{};

        explicit ConstantClassNameTypeIndexInfo(std::istream &is) : ConstantInfo(is) {
            read(classIndex, is);
            read(nameAndTypeIndex, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}.#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), classIndex,
                          nameAndTypeIndex);
        }
    };

    struct ConstantFieldInfo : public ConstantClassNameTypeIndexInfo {
        explicit ConstantFieldInfo(std::istream &is) : ConstantClassNameTypeIndexInfo(is) {
        }
    };

    struct ConstantMethodInfo : public ConstantClassNameTypeIndexInfo {
        explicit ConstantMethodInfo(std::istream &is) : ConstantClassNameTypeIndexInfo(is) {
        }
    };

    struct ConstantInterfaceMethodInfo : public ConstantClassNameTypeIndexInfo {
        explicit ConstantInterfaceMethodInfo(std::istream &is) : ConstantClassNameTypeIndexInfo(is) {
        }
    };


    struct ConstantIntegerInfo : public ConstantInfo {
        i4 value;

        explicit ConstantIntegerInfo(std::istream &is) : ConstantInfo(is) {
            value = static_cast<i4>(read<u4>(is));
        }

        cstring toString() override {
            return cformat("{:<20}{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), value);
        }
    };

    struct ConstantFloatInfo : public ConstantInfo {
        f4 value;

        explicit ConstantFloatInfo(std::istream &is) : ConstantInfo(is) {
            StreamByteType auto i = read<u4>(is);
            //value = *reinterpret_cast<f4 *>(&i);

            value = std::bit_cast<f4>(i);
        }

        cstring toString() override {
            return cformat("{:<20}{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), value);
        }
    };

    struct ConstantLongInfo : public ConstantInfo {
        i8 value;

        explicit ConstantLongInfo(std::istream &is) : ConstantInfo(is) {
            value = static_cast<i8>(read<u8>(is));
        }

        cstring toString() override {
            return cformat("{:<20}{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), value);
        }
    };

    struct ConstantDoubleInfo : public ConstantInfo {
        f8 value;

        explicit ConstantDoubleInfo(std::istream &is) : ConstantInfo(is) {
            u8 i = read<u8>(is);
            //type1
            // union { u8 t1; f8 t2; } u1 = { .t1 = i};
            // value = u1.t2;

            //type2
            // value = *reinterpret_cast<f8 *>(&i);

            //type 3
            value = std::bit_cast<f8>(i);
        }

        cstring toString() override {
            return cformat("{:<20}{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), value);
        }
    };

    struct ConstantNameAndTypeInfo : public ConstantInfo {
        u2 nameIndex{};
        u2 descriptorIndex{};

        explicit ConstantNameAndTypeInfo(std::istream &is) : ConstantInfo(is) {
            read(nameIndex, is);
            read(descriptorIndex, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}:#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), nameIndex,
                          descriptorIndex);
        }
    };

    struct ConstantUTF8Info : public ConstantInfo {
        u2 length{};
        std::unique_ptr<u1[]> bytes;
        cstring str;

        explicit ConstantUTF8Info(std::istream &is) : ConstantInfo(is) {
            read(length, is);
            bytes = readBuffer(is, length);
            str = cstring(reinterpret_cast<char *>(bytes.get()), length);
        }

        cstring toString() override {
            return cformat("{:<20}{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), str);
        }
    };

    struct ConstantMethodHandleInfo : public ConstantInfo {
        u1 referenceKind{};
        u2 referenceIndex{};

        explicit ConstantMethodHandleInfo(std::istream &is) : ConstantInfo(is) {
            read(referenceKind, is);
            read(referenceIndex, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}:#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), referenceKind,
                          referenceIndex);
        }
    };

    struct ConstantMethodTypeInfo : public ConstantInfo {
        u2 descriptorIndex{};

        explicit ConstantMethodTypeInfo(std::istream &is) : ConstantInfo(is) {
            read(descriptorIndex, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)), descriptorIndex);
        }
    };

    struct ConstantInvokeDynamicInfo : public ConstantInfo {
        u2 bootstrapMethodAttrIndex{};
        u2 nameAndTypeIndex{};

        explicit ConstantInvokeDynamicInfo(std::istream &is) : ConstantInfo(is) {
            read(bootstrapMethodAttrIndex, is);
            read(nameAndTypeIndex, is);
        }

        cstring toString() override {
            return cformat("{:<20}#{}:#{}", CONST_NAME_MAP.at(static_cast<ConstantTagEnum>(tag)),
                          bootstrapMethodAttrIndex, nameAndTypeIndex);
        }
    };

    inline cstring getConstantString(ConstantInfo *info) {
        return dynamic_cast<ConstantUTF8Info *>(info)->str;
    }

    inline std::tuple<const u1 *, u2> getConstantStringBytes(ConstantInfo *info) {
        auto utf8info = dynamic_cast<ConstantUTF8Info *>(info);
        return std::make_tuple(utf8info->bytes.get(), utf8info->length);
    }

    inline cstring
    getConstantStringFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        return getConstantString(pool.at(index).get());
    }

    inline std::tuple<const u1 *, u2>
    getConstantStringBytesFromPool(const std::vector<std::unique_ptr<ConstantInfo>> &pool, const size_t index) {
        return getConstantStringBytes(pool.at(index).get());
    }


}

#endif