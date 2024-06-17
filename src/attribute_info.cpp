#include "attribute_info.hpp"
#include "constantInfo.hpp"

namespace RexVM {

    const std::unordered_map<cstring, AttributeTagEnum> ATTRIBUTE_NAME_TAG_MAP{
            {"ConstantValue",                        AttributeTagEnum::CONSTANT_VALUE},
            {"Code",                                 AttributeTagEnum::CODE},
            {"StackMapTable",                        AttributeTagEnum::STACK_MAP_TABLE},
            {"Exceptions",                           AttributeTagEnum::EXCEPTIONS},
            {"InnerClasses",                         AttributeTagEnum::INNER_CLASSES},
            {"EnclosingMethod",                      AttributeTagEnum::ENCLOSING_METHOD},
            {"Synthetic",                            AttributeTagEnum::SYNTHETIC},
            {"Signature",                            AttributeTagEnum::SIGNATURE},
            {"SourceFile",                           AttributeTagEnum::SOURCE_FILE},
            {"SourceDebugExtension",                 AttributeTagEnum::SOURCE_DEBUG_EXTENSION},
            {"LineNumberTable",                      AttributeTagEnum::LINE_NUMBER_TABLE},
            {"LocalVariableTable",                   AttributeTagEnum::LOCAL_VARIABLE_TABLE},
            {"LocalVariableTypeTable",               AttributeTagEnum::LOCAL_VARIABLE_TYPE_TABLE},
            {"Deprecated",                           AttributeTagEnum::DEPRECATED},
            {"RuntimeVisibleAnnotations",            AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS},
            {"RuntimeInvisibleAnnotations",          AttributeTagEnum::RUNTIME_INVISIBLE_ANNOTATIONS},
            {"RuntimeVisibleParameterAnnotations",   AttributeTagEnum::RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS},
            {"RuntimeInvisibleParameterAnnotations", AttributeTagEnum::RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS},
            {"RuntimeVisibleTypeAnnotations",        AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS},
            {"RuntimeInvisibleTypeAnnotations",      AttributeTagEnum::RUNTIME_INVISIBLE_TYPE_ANNOTATIONS},
            {"AnnotationDefault",                    AttributeTagEnum::ANNOTATION_DEFAULT},
            {"BootstrapMethods",                     AttributeTagEnum::BOOTSTRAP_METHODS},
            {"MethodParameters",                     AttributeTagEnum::METHOD_PARAMETERS},
            {"NestMembers",                          AttributeTagEnum::NEST_MEMBER},
            {"NestHost",                             AttributeTagEnum::NEST_HOST},
    };


    std::unique_ptr<AttributeInfo>
    parseAttribute(std::istream &is, const std::vector<std::unique_ptr<ConstantInfo>> &constantInfoPool) {
        const StreamByteType auto attributeNameIndex = peek<u2>(is);
        const auto attributeName = getConstantStringFromPool(constantInfoPool, (const size_t) attributeNameIndex);
        const auto attrIter = ATTRIBUTE_NAME_TAG_MAP.find(attributeName);
        if (attrIter == ATTRIBUTE_NAME_TAG_MAP.end()) {
            //not found, not implement
            //return std::make_unique<SkipAttribute>(is); 
            panic("not implement attribute " + attributeName);
        }
        const auto tagEnum = attrIter->second;
        switch (tagEnum) {
            case AttributeTagEnum::CONSTANT_VALUE:
                return std::make_unique<ConstantValueAttribute>(is);

            case AttributeTagEnum::CODE:
                return std::make_unique<CodeAttribute>(is, constantInfoPool);

            case AttributeTagEnum::STACK_MAP_TABLE:
                //return std::make_unique<StackMapTableAttribute>(is);
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::EXCEPTIONS:
                return std::make_unique<ExceptionsAttribute>(is);

            case AttributeTagEnum::INNER_CLASSES:
                return std::make_unique<InnerClassesAttribute>(is);

            case AttributeTagEnum::ENCLOSING_METHOD:
                return std::make_unique<EnclosingMethodAttribute>(is);

            case AttributeTagEnum::SYNTHETIC:
                return std::make_unique<SyntheticAttribute>(is);

            case AttributeTagEnum::SIGNATURE:
                return std::make_unique<SignatureAttribute>(is);

            case AttributeTagEnum::SOURCE_FILE:
                return std::make_unique<SourceFileAttribute>(is);

            case AttributeTagEnum::SOURCE_DEBUG_EXTENSION:
                //return std::make_unique<SourceDebugExtensionAttribute>(is);
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::LINE_NUMBER_TABLE:
                return std::make_unique<LineNumberTableAttribute>(is);

            case AttributeTagEnum::LOCAL_VARIABLE_TABLE:
                return std::make_unique<LocalVariableTableAttribute>(is);

            case AttributeTagEnum::LOCAL_VARIABLE_TYPE_TABLE:
                return std::make_unique<LocalVariableTypeTableAttribute>(is);

            case AttributeTagEnum::DEPRECATED:
                return std::make_unique<DeprecatedAttribute>(is);

            case AttributeTagEnum::BOOTSTRAP_METHODS:
                return std::make_unique<BootstrapMethodsAttribute>(is);

            case AttributeTagEnum::METHOD_PARAMETERS:
                return std::make_unique<MethodParametersAttribute>(is);

            case AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS:
                //return std::make_unique<RuntimeVisibleAnnotationsAttribute>(is);
                return std::make_unique<ByteStreamAttribute>(is);

            case AttributeTagEnum::RUNTIME_INVISIBLE_ANNOTATIONS:
                //return std::make_unique<RuntimeInvisibleAnnotationsAttribute>(is);
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS:
                //return std::make_unique<RuntimeVisibleParameterAnnotationsAttribute>(is);
                return std::make_unique<ByteStreamAttribute>(is);

            case AttributeTagEnum::RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS:
                //return std::make_unique<RuntimeInvisibleParameterAnnotationsAttribute>(is);
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS:
                //return std::make_unique<RuntimeVisibleTypeAnnotationsAttribute>(is);
                return std::make_unique<ByteStreamAttribute>(is);

            case AttributeTagEnum::RUNTIME_INVISIBLE_TYPE_ANNOTATIONS:
                //return std::make_unique<RuntimeInvisibleTypeAnnotationsAttribute>(is);
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::ANNOTATION_DEFAULT:
                //return std::make_unique<AnnotationDefaultAttribute>(is);
                return std::make_unique<ByteStreamAttribute>(is);

            case AttributeTagEnum::NEST_MEMBER:
                return std::make_unique<SkipAttribute>(is);

            case AttributeTagEnum::NEST_HOST:
                return std::make_unique<SkipAttribute>(is);

            default:
                panic(format("parseAttribute error attributeNameIndex {}", attributeNameIndex));
                break;
        }
        return nullptr;
    }

    AttributeInfo *getAssignAttribute(
            const std::vector<std::unique_ptr<ConstantInfo>> &constantPool,
            const std::vector<std::unique_ptr<AttributeInfo>> &attributes,
            AttributeTagEnum tagEnum
    ) {
        for (auto &attribute: attributes) {
            const auto attributeName =
                    getConstantStringFromPool(constantPool, (const size_t) attribute->attributeNameIndex);
            const auto currentTagEnum = ATTRIBUTE_NAME_TAG_MAP.at(attributeName);
            if (tagEnum == currentTagEnum) {
                return attribute.get();
            }
        }
        return nullptr;
    }

    ElementValue::ElementValue(std::istream &is) {
        read(tag, is);
        switch ((char) tag) {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
            case 's':
                value = std::make_unique<ConstValue>(is);
                break;

            case 'e':
                value = std::make_unique<EnumConstValue>(is);
                break;

            case 'c':
                value = std::make_unique<ClassInfoValue>(is);
                break;

            case '@':
                value = std::make_unique<Annotation>(is);
                break;

            case '[':
                value = std::make_unique<ArrayValue>(is);
                break;

            default:
                panic(format("createVerificationTypeInfo error {}", tag));
                break;
        }
        push(tag);
        push(value->stub);
    }

    std::unique_ptr<StackMapTableAttribute::VerificationTypeInfo>
    StackMapTableAttribute::createVerificationTypeInfo(std::istream &is) {
        const StreamByteType auto verificationTag = peek<u1>(is);
        const auto verificationTagEnum = static_cast<VerificationTypeTagEnum>(verificationTag);

        switch (verificationTagEnum) {
            case VerificationTypeTagEnum::ITEM_Top:
                return std::make_unique<TopVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Integer:
                return std::make_unique<IntegerVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Float:
                return std::make_unique<FloatVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Double:
                return std::make_unique<DoubleVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Long:
                return std::make_unique<LongVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Null:
                return std::make_unique<NullVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_UninitializedThis:
                return std::make_unique<UninitializedThisVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Object:
                return std::make_unique<ObjectVariableInfo>(is);

            case VerificationTypeTagEnum::ITEM_Uninitialized:
                return std::make_unique<UninitializedVariableInfo>(is);

            default:
                panic(fmt::format("createVerificationTypeInfo error {}", verificationTag));
                break;
        }
    }
}
