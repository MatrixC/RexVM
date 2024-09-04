#include "utils/format.hpp"
#include "attribute_info.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"

namespace RexVM {

    FMBaseInfo::FMBaseInfo(std::istream &is, ClassFile &cf) : cf(cf) {
        read(accessFlags, is);
        read(nameIndex, is);
        read(descriptorIndex, is);
        read(attributesCount, is);
        for (auto i = 0; i < attributesCount; ++i) {
            attributes.emplace_back(parseAttribute(is, cf.constantPool));
        }
    }

    FMBaseInfo::~FMBaseInfo() = default;

    cview FMBaseInfo::getName() const {
        return getConstantStringFromPool(cf.constantPool, nameIndex);
    }

    cview FMBaseInfo::getDescriptor() const {
        return getConstantStringFromPool(cf.constantPool, descriptorIndex);
    }

    AttributeInfo *FMBaseInfo::getAssignAttribute(AttributeTagEnum tagEnum) const {
        return getAssignAttributeByConstantPool(cf.constantPool, attributes, tagEnum);
    }

    ClassFile::ClassFile(std::istream &is) {
        parserAll(is);
    }

    ClassFile::~ClassFile() = default;

    void ClassFile::parseHeader(std::istream &is) {
        read(magic, is);
        read(minorVersion, is);
        read(majorVersion, is);
    }

    void ClassFile::parseConstantPool(std::istream &is) {
        read(constantPoolCount, is);
        //Class file specification, constantPool first empty
        constantPool.reserve(constantPoolCount);
        constantPool.emplace_back(nullptr);
        for (auto i = 1; i < constantPoolCount; ++i) {
            const StreamByteType auto tag = peek<u1>(is);
            const auto constantTagEnum = CAST_CONSTANT_TAG_ENUM(tag);
            switch (constantTagEnum) {
                case ConstantTagEnum::CONSTANT_Class:
                    constantPool.emplace_back(std::make_unique<ConstantClassInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_String:
                    constantPool.emplace_back(std::make_unique<ConstantStringInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_FieldRef:
                    constantPool.emplace_back(std::make_unique<ConstantFieldInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_MethodRef:
                    constantPool.emplace_back(std::make_unique<ConstantMethodInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_InterfaceMethodRef:
                    constantPool.emplace_back(std::make_unique<ConstantInterfaceMethodInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_Integer:
                    constantPool.emplace_back(std::make_unique<ConstantIntegerInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_Float:
                    constantPool.emplace_back(std::make_unique<ConstantFloatInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_Long:
                    constantPool.emplace_back(std::make_unique<ConstantLongInfo>(is));
                    constantPool.emplace_back(nullptr);
                    ++i;
                    break;

                case ConstantTagEnum::CONSTANT_Double:
                    constantPool.emplace_back(std::make_unique<ConstantDoubleInfo>(is));
                    constantPool.emplace_back(nullptr);
                    ++i;
                    break;

                case ConstantTagEnum::CONSTANT_NameAndType:
                    constantPool.emplace_back(std::make_unique<ConstantNameAndTypeInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_Utf8:
                    constantPool.emplace_back(std::make_unique<ConstantUTF8Info>(is));
                    break;

                case ConstantTagEnum::CONSTANT_MethodHandle:
                    constantPool.emplace_back(std::make_unique<ConstantMethodHandleInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_MethodType:
                    constantPool.emplace_back(std::make_unique<ConstantMethodTypeInfo>(is));
                    break;

                case ConstantTagEnum::CONSTANT_InvokeDynamic:
                    constantPool.emplace_back(std::make_unique<ConstantInvokeDynamicInfo>(is));
                    break;

                default:
                    panic(cformat("parseConstantPool error tag {}", tag));
                    break;
            }
        }
    }

    void ClassFile::parseClassInfo(std::istream &is) {
        read(accessFlags, is);
        read(thisClass, is);
        read(superClass, is);
    }

    void ClassFile::parseInterfaces(std::istream &is) {
        read(interfaceCount, is);
        if (interfaceCount > 0) {
            for (auto i = 0; i < interfaceCount; ++i) {
                interfaces.emplace_back(read<u2>(is));
            }
        }
    }

    void ClassFile::parseFields(std::istream &is) {
        read(fieldCount, is);
        for (auto i = 0; i < fieldCount; ++i) {
            fields.emplace_back(std::make_unique<FieldInfo>(is, *this));
        }
    }

    void ClassFile::parseMethods(std::istream &is) {
        read(methodCount, is);
        for (auto i = 0; i < methodCount; ++i) {
            methods.emplace_back(std::make_unique<MethodInfo>(is, *this));
        }
    }

    void ClassFile::parseClassAttributes(std::istream &is) {
        read(attributeCount, is);
        for (auto i = 0; i < attributeCount; ++i) {
            attributes.emplace_back(parseAttribute(is, constantPool));
        }
    }

    void ClassFile::parserAll(std::istream &is) {
        parseHeader(is);
        parseConstantPool(is);
        parseClassInfo(is);
        parseInterfaces(is);
        parseFields(is);
        parseMethods(is);
        parseClassAttributes(is);
    }

    AttributeInfo *ClassFile::getAssignAttribute(AttributeTagEnum tagEnum) const {
        return getAssignAttributeByConstantPool(constantPool, attributes, tagEnum);
    }

    cview ClassFile::getClassName(u2 classIndex) const {
        auto classInfo = CAST_CONSTANT_CLASS_INFO(constantPool[classIndex].get());
        return getConstantStringFromPool(constantPool, classInfo->index);
    }

    cview ClassFile::getThisClassName() const {
        return getClassName(thisClass);
    }

    cview ClassFile::getSuperClassName() const {
        if (superClass == 0) {
            return {};
        }
        return getClassName(superClass);
    }

    cview ClassFile::getSourceFile() const {
        const auto sourceFileAttribute = getAssignAttribute(AttributeTagEnum::SOURCE_FILE);

        if (sourceFileAttribute == nullptr) {
            return cview("");
        }

        const auto nameIndex = (CAST_SOURCE_FILE_ATTRIBUTE(sourceFileAttribute))->sourceFileIndex;
        return getConstantStringFromPool(constantPool, nameIndex);
    }

    cview ClassFile::getSignature() const {
        const auto signatureAttribute = getAssignAttribute(AttributeTagEnum::SIGNATURE);

        if (signatureAttribute == nullptr) {
            return {};
        }

        const auto nameIndex = (CAST_SIGNATURE_ATTRIBUTE(signatureAttribute))->signatureIndex;
        return getConstantStringFromPool(constantPool, nameIndex);    
    }

    void ClassFile::getBootstrapMethods() const {
        const auto oriAttribute = getAssignAttribute(AttributeTagEnum::BOOTSTRAP_METHODS);

        if (oriAttribute == nullptr) {
            return;
        }
    }

    std::vector<cview> ClassFile::getInterfaceNames() const {
        std::vector<cview> list;
        list.reserve(interfaces.size());
        for (auto interfaceIndex: interfaces) {
            list.emplace_back(getClassName(interfaceIndex));
        }
        return list;
    }
}