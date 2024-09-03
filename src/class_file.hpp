#ifndef CLASS_FILE_HPP
#define CLASS_FILE_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "config.hpp"
#include "attribute_info.hpp"
#include "composite_string.hpp"

namespace RexVM {

    constexpr std::uint32_t MAGIC_NUMBER = 0xCAFEBABE;

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

        [[nodiscard]] cview getName() const;

        [[nodiscard]] cview getDescriptor() const;

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

    using ConstantPoolRef = std::vector<std::unique_ptr<ConstantInfo>> &;

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

        [[nodiscard]] AttributeInfo *getAssignAttribute(AttributeTagEnum tagEnum) const;

        [[nodiscard]] cview getClassName(u2 classIndex) const;

        [[nodiscard]] cview getThisClassName() const;

        [[nodiscard]] cview getSuperClassName() const;

        [[nodiscard]] rstring getSourceFile() const;
        
        [[nodiscard]] cview getSignature() const;

        void getBootstrapMethods() const;

        [[nodiscard]] std::vector<cstring> getInterfaceNames() const;
    };


}

#endif //CLASS_FILE_HPP
