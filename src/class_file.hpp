//
// Created by ch on 12/17/23.
//

#ifndef CLASS_FILE_HPP
#define CLASS_FILE_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "config.hpp"
#include "attribute_info.hpp"

namespace RexVM {

    constexpr std::uint32_t MAGIC_NUMBER = 0xCAFEBABE;
    enum class AccessFlagEnum : u2 {
        ACC_PUBLIC = 0x0001,
        ACC_PRIVATE = 0x0002,
        ACC_PROTECTED = 0x0004,
        ACC_STATIC = 0x0008,
        ACC_FINAL = 0x0010,
        ACC_SUPER = 0x0020,
        ACC_SYNCHRONIZED = 0x0020,
        ACC_VOLATILE = 0x0040,
        ACC_BRIDGE = 0x0040,
        ACC_TRANSIENT = 0x0080,
        ACC_VARARGS = 0x0080,
        ACC_NATIVE = 0x0100,
        ACC_INTERFACE = 0x0200,
        ACC_ABSTRACT = 0x0400,
        ACC_STRICT = 0x0800,
        ACC_SYNTHETIC = 0x1000,
        ACC_ANNOTATION = 0x2000,
        ACC_ENUM = 0x4000,
        ACC_MODULE = 0x8000,
        ACC_MANDATED = 0x8000,
    };

    struct ConstantInfo;
    struct AttributeInfo;
    struct ClassFile;

    struct FMBaseInfo {
        u2 accessFlags{};
        u2 nameIndex{};
        u2 descriptorIndex{};
        u2 attributesCount{};
        std::vector<std::unique_ptr<AttributeInfo>> attributes;
        ClassFile &cf;

        explicit FMBaseInfo(std::istream &is, ClassFile &cf);

        ~FMBaseInfo();

        [[nodiscard]] cstring getName() const;

        [[nodiscard]] cstring getDescriptor() const;

        [[nodiscard]] AttributeInfo *getAssignAttribute(AttributeTagEnum tagEnum) const;
    };

    struct FieldInfo : public FMBaseInfo {
        explicit FieldInfo(std::istream &is, ClassFile &cf) : FMBaseInfo(is, cf) {
        }
    };

    struct MethodInfo : public FMBaseInfo {
        explicit MethodInfo(std::istream &is, ClassFile &cf) : FMBaseInfo(is, cf) {
        }
    };

    struct ClassFile {
        u4 magic{};
        u2 minorVersion{};
        u2 majorVersion{};
        u2 constantPoolCount{};
        std::vector<std::unique_ptr<ConstantInfo>> constantPool;
        u2 accessFlags{};
        u2 thisClass{};
        u2 superClass{};
        u2 interfaceCount{};
        std::vector<u2> interfaces;
        u2 fieldCount{};
        std::vector<std::unique_ptr<FieldInfo>> fields;
        u2 methodCount{};
        std::vector<std::unique_ptr<MethodInfo>> methods;
        u2 attributeCount{};
        std::vector<std::unique_ptr<AttributeInfo>> attributes;

        explicit ClassFile() = default;

        ClassFile(const ClassFile &) = delete;

        ClassFile(ClassFile &&) = delete;


        void parseHeader(std::istream &is);

        void parseConstantPool(std::istream &is);

        void parseClassInfo(std::istream &is);

        void parseInterfaces(std::istream &is);

        void parseFields(std::istream &is);

        void parseMethods(std::istream &is);

        void parseClassAttributes(std::istream &is);

        void parserAll(std::istream &is);

        explicit ClassFile(std::istream &is);

        ~ClassFile();

        [[nodiscard]] cstring getClassName(u2 classIndex) const;

        [[nodiscard]] cstring getThisClassName() const;

        [[nodiscard]] cstring getSuperClassName() const;

        [[nodiscard]] std::vector<cstring> getInterfaceNames() const;
    };


}

#endif //CLASS_FILE_HPP
