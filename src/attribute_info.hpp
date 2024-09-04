#ifndef ATTRIBUTE_INFO_HPP
#define ATTRIBUTE_INFO_HPP

#include <vector>
#include <memory>
#include <hash_table8.hpp>
#include "config.hpp"
#include "utils/binary.hpp"
#include "utils/format.hpp"
#include "exception.hpp"

namespace RexVM {

    struct ConstantInfo;
    struct AttributeInfo;

    enum class AttributeTagEnum : std::uint8_t {
        CONSTANT_VALUE = 0,
        CODE = 1,
        STACK_MAP_TABLE = 2,
        EXCEPTIONS = 3,
        INNER_CLASSES = 4,
        ENCLOSING_METHOD = 5,
        SYNTHETIC = 6,
        SIGNATURE = 7,
        SOURCE_FILE = 8,
        SOURCE_DEBUG_EXTENSION = 9,
        LINE_NUMBER_TABLE = 10,
        LOCAL_VARIABLE_TABLE = 11,
        LOCAL_VARIABLE_TYPE_TABLE = 12,
        DEPRECATED = 13,
        RUNTIME_VISIBLE_ANNOTATIONS = 14,
        RUNTIME_INVISIBLE_ANNOTATIONS = 15,
        RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS = 16,
        RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS = 17,
        RUNTIME_VISIBLE_TYPE_ANNOTATIONS = 18,
        RUNTIME_INVISIBLE_TYPE_ANNOTATIONS = 19,
        ANNOTATION_DEFAULT = 20,
        BOOTSTRAP_METHODS = 21,
        METHOD_PARAMETERS = 22,
        NEST_MEMBER = 23,
        NEST_HOST = 24,
    };

    extern const emhash8::HashMap<cview, AttributeTagEnum> ATTRIBUTE_NAME_TAG_MAP;

    AttributeInfo *getAssignAttributeByConstantPool(
            const std::vector<std::unique_ptr<ConstantInfo>> &constantPool,
            const std::vector<std::unique_ptr<AttributeInfo>> &attributes,
            AttributeTagEnum tagEnum
    );

    std::unique_ptr<AttributeInfo>
    parseAttribute(std::istream &is, const std::vector<std::unique_ptr<ConstantInfo>> &constantInfoPool);

    struct AttributeInfo {
        u2 attributeNameIndex{};
        u4 attributeLength{};

        explicit AttributeInfo(std::istream &is) {
            read(attributeNameIndex, is);
            read(attributeLength, is);
        }

        virtual ~AttributeInfo() = default;
    };

    struct SkipAttribute : public AttributeInfo {
        explicit SkipAttribute(std::istream &is) : AttributeInfo(is) {
            is.seekg(attributeLength, std::ios::cur);
        }
    };

    struct ExceptionTable {
        u2 startPC{};
        u2 endPC{};
        u2 handlerPC{};
        u2 catchType{};

        explicit ExceptionTable(std::istream &is) {
            read(startPC, is);
            read(endPC, is);
            read(handlerPC, is);
            read(catchType, is);
        }
    };

    struct CodeAttribute : public AttributeInfo {

        u2 maxStack{};
        u2 maxLocals{};
        u4 codeLength{};
        std::unique_ptr<u1[]> code;
        u2 exceptionTableLength{};
        std::vector<std::unique_ptr<ExceptionTable>> exceptionTables;
        u2 attributesCount{};
        std::vector<std::unique_ptr<AttributeInfo>> attributes;

        explicit CodeAttribute(std::istream &is, const std::vector<std::unique_ptr<ConstantInfo>> &constantInfoPool)
                : AttributeInfo(is) {
            read(maxStack, is);
            read(maxLocals, is);
            read(codeLength, is);
            if (codeLength > 0) {
                code = readBuffer(is, codeLength);
            }
            read(exceptionTableLength, is);
            for (auto i = 0; i < exceptionTableLength; ++i) {
                exceptionTables.emplace_back(std::make_unique<ExceptionTable>(is));
            }
            read(attributesCount, is);
            for (auto i = 0; i < attributesCount; ++i) {
                attributes.emplace_back(parseAttribute(is, constantInfoPool));
            }
        }
    };

    struct ConstantValueAttribute : public AttributeInfo {
        u2 constantValueIndex{};

        explicit ConstantValueAttribute(std::istream &is) : AttributeInfo(is) {
            read(constantValueIndex, is);
        }
    };

    struct DeprecatedAttribute : public AttributeInfo {
        explicit DeprecatedAttribute(std::istream &is) : AttributeInfo(is) {
        }
    };

    struct SyntheticAttribute : public AttributeInfo {
        explicit SyntheticAttribute(std::istream &is) : AttributeInfo(is) {
        }
    };

    struct ExceptionsAttribute : public AttributeInfo {
        u2 numberOfExceptions{};
        std::vector<u2> exceptionIndexTable;

        explicit ExceptionsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfExceptions, is);
            for (auto i = 0; i < numberOfExceptions; ++i) {
                exceptionIndexTable.emplace_back(read<u2>(is));
            }
        }
    };

    struct EnclosingMethodAttribute : public AttributeInfo {
        u2 classIndex{};
        u2 methodIndex{};

        explicit EnclosingMethodAttribute(std::istream &is) : AttributeInfo(is) {
            read(classIndex, is);
            read(methodIndex, is);
        }
    };

    struct InnerClassesAttribute : public AttributeInfo {
        struct InnerClassesInfo {
            u2 innerClassInfoIndex{};
            u2 outerClassInfoIndex{};
            u2 innerNameIndex{};
            u2 innerClassAccessFlags{};

            explicit InnerClassesInfo(std::istream &is) {
                read(innerClassInfoIndex, is);
                read(outerClassInfoIndex, is);
                read(innerNameIndex, is);
                read(innerClassAccessFlags, is);
            }
        };

        u2 numberOfInnerClasses{};
        std::vector<std::unique_ptr<InnerClassesInfo>> classes;

        explicit InnerClassesAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfInnerClasses, is);
            for (auto i = 0; i < numberOfInnerClasses; ++i) {
                classes.emplace_back(std::make_unique<InnerClassesInfo>(is));
            }
        }
    };

    struct LineNumberInfo {
        u2 startPC{};
        u2 lineNumber{};

        explicit LineNumberInfo(std::istream &is) {
            read(startPC, is);
            read(lineNumber, is);
        }
    };

    struct LineNumberTableAttribute : public AttributeInfo {
        u2 lineNumberTableLength{};
        std::vector<std::unique_ptr<LineNumberInfo>> lineNumberTables;

        explicit LineNumberTableAttribute(std::istream &is) : AttributeInfo(is) {
            read(lineNumberTableLength, is);
            for (auto i = 0; i < lineNumberTableLength; ++i) {
                lineNumberTables.emplace_back(std::make_unique<LineNumberInfo>(is));
            }
        }
    };

    struct LocalVariableTableAttribute : public AttributeInfo {
        struct LocalVariableInfo {
            u2 startPC{};
            u2 length{};
            u2 nameIndex{};
            u2 descriptorIndex{};
            u2 index{};

            explicit LocalVariableInfo(std::istream &is) {
                read(startPC, is);
                read(length, is);
                read(nameIndex, is);
                read(descriptorIndex, is);
                read(index, is);
            }
        };

        u2 localVariableTableLength{};
        std::vector<std::unique_ptr<LocalVariableInfo>> localVariableTables;

        explicit LocalVariableTableAttribute(std::istream &is) : AttributeInfo(is) {
            read(localVariableTableLength, is);
            for (auto i = 0; i < localVariableTableLength; ++i) {
                localVariableTables.emplace_back(std::make_unique<LocalVariableInfo>(is));
            }
        }
    };

    struct LocalVariableTypeTableAttribute : public AttributeInfo {
        struct LocalVariableTypeInfo {
            u2 startPC{};
            u2 length{};
            u2 nameIndex{};
            u2 signatureIndex{};
            u2 index{};

            explicit LocalVariableTypeInfo(std::istream &is) {
                read(startPC, is);
                read(length, is);
                read(nameIndex, is);
                read(signatureIndex, is);
                read(index, is);
            }
        };

        u2 localVariableTypeTableLength{};
        std::vector<std::unique_ptr<LocalVariableTypeInfo>> localVariableTypeTables;

        explicit LocalVariableTypeTableAttribute(std::istream &is) : AttributeInfo(is) {
            read(localVariableTypeTableLength, is);
            for (auto i = 0; i < localVariableTypeTableLength; ++i) {
                localVariableTypeTables.emplace_back(std::make_unique<LocalVariableTypeInfo>(is));
            }
        }
    };

    struct SignatureAttribute : public AttributeInfo {
        u2 signatureIndex{};

        explicit SignatureAttribute(std::istream &is) : AttributeInfo(is) {
            read(signatureIndex, is);
        }
    };

    struct SourceFileAttribute : public AttributeInfo {
        u2 sourceFileIndex{};

        explicit SourceFileAttribute(std::istream &is) : AttributeInfo(is) {
            read(sourceFileIndex, is);
        }
    };

    struct SourceDebugExtensionAttribute : public AttributeInfo {
        std::unique_ptr<u1[]> debugExtension;

        explicit SourceDebugExtensionAttribute(std::istream &is) : AttributeInfo(is) {
            if (attributeLength > 0) {
                debugExtension = readBuffer(is, attributeLength);
            }
        }
    };

    struct BootstrapMethodsAttribute : public AttributeInfo {
        struct BootstrapMethod {
            u2 bootstrapMethodRef{};
            u2 numberOfBootstrapArguments{};
            std::vector<u2> bootstrapArguments;

            explicit BootstrapMethod(std::istream &is) {
                read(bootstrapMethodRef, is);
                read(numberOfBootstrapArguments, is);
                for (auto i = 0l; i < numberOfBootstrapArguments; ++i) {
                    bootstrapArguments.emplace_back(read<u2>(is));
                }
            }
        };

        u2 numberOfBootstrapMethods{};
        std::vector<std::unique_ptr<BootstrapMethod>> bootstrapMethods;

        explicit BootstrapMethodsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfBootstrapMethods, is);
            for (auto i = 0; i < numberOfBootstrapMethods; ++i) {
                bootstrapMethods.emplace_back(std::make_unique<BootstrapMethod>(is));
            }
        }
    };

    struct MethodParametersAttribute : public AttributeInfo {
        struct Parameter {
            u2 nameIndex{};
            u2 accessFlags{};

            explicit Parameter(std::istream &is) {
                read(nameIndex, is);
                read(accessFlags, is);
            }
        };

        u1 numberOfParameters{};
        std::vector<std::unique_ptr<Parameter>> parameters;

        explicit MethodParametersAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfParameters, is);
            for (auto i = 0; i < numberOfParameters; ++i) {
                parameters.emplace_back(std::make_unique<Parameter>(is));
            }
        }

    };

    struct StackMapTableAttribute : public AttributeInfo {

        enum class VerificationTypeTagEnum : std::uint8_t {
            ITEM_Top = 0,
            ITEM_Integer = 1,
            ITEM_Float = 2,
            ITEM_Double = 3,
            ITEM_Long = 4,
            ITEM_Null = 5,
            ITEM_UninitializedThis = 6,
            ITEM_Object = 7,
            ITEM_Uninitialized = 8,
        };

        struct VerificationTypeInfo {
            u1 tag{};

            explicit VerificationTypeInfo(std::istream &is) {
                read(tag, is);
            }

            virtual ~VerificationTypeInfo() = default;

        };

        static std::unique_ptr<VerificationTypeInfo> createVerificationTypeInfo(std::istream &is);

        struct TopVariableInfo : public VerificationTypeInfo {
            explicit TopVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct IntegerVariableInfo : public VerificationTypeInfo {
            explicit IntegerVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct FloatVariableInfo : public VerificationTypeInfo {
            explicit FloatVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct DoubleVariableInfo : public VerificationTypeInfo {
            explicit DoubleVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct LongVariableInfo : public VerificationTypeInfo {
            explicit LongVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct NullVariableInfo : public VerificationTypeInfo {
            explicit NullVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct UninitializedThisVariableInfo : public VerificationTypeInfo {
            explicit UninitializedThisVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
            }
        };

        struct ObjectVariableInfo : public VerificationTypeInfo {
            u2 cPoolIndex{};

            explicit ObjectVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
                read(cPoolIndex, is);
            }
        };

        struct UninitializedVariableInfo : public VerificationTypeInfo {
            u2 offset{};

            explicit UninitializedVariableInfo(std::istream &is) : VerificationTypeInfo(is) {
                read(offset, is);
            }
        };


        struct StackMapFrame {
            u1 frameType{};

            explicit StackMapFrame(std::istream &is) {
                read(frameType, is);
            }

            virtual ~StackMapFrame() = default;

        };

        struct SameFrame : public StackMapFrame {
            explicit SameFrame(std::istream &is) : StackMapFrame(is) {}
        };

        struct SameLocals1StackItemFrame : public StackMapFrame {
            std::vector<std::unique_ptr<VerificationTypeInfo>> stack;

            explicit SameLocals1StackItemFrame(std::istream &is) : StackMapFrame(is) {
                stack.emplace_back(createVerificationTypeInfo(is));
            }
        };

        struct SameLocals1StackItemFrameExtended : public StackMapFrame {
            u2 offsetDelta{};
            std::vector<std::unique_ptr<VerificationTypeInfo>> stack;

            explicit SameLocals1StackItemFrameExtended(std::istream &is) : StackMapFrame(is) {
                read(offsetDelta, is);
                stack.emplace_back(createVerificationTypeInfo(is));
            }
        };

        struct ChopFrame : public StackMapFrame {
            u2 offsetDelta{};

            explicit ChopFrame(std::istream &is) : StackMapFrame(is) {
                read(offsetDelta, is);
            }
        };

        struct SameFrameExtended : public StackMapFrame {
            u2 offsetDelta{};

            explicit SameFrameExtended(std::istream &is) : StackMapFrame(is) {
                read(offsetDelta, is);
            }
        };

        struct AppendFrame : public StackMapFrame {
            u2 offsetDelta{};
            std::vector<std::unique_ptr<VerificationTypeInfo>> locals;

            explicit AppendFrame(std::istream &is) : StackMapFrame(is) {
                read(offsetDelta, is);
                for (auto i = 0; i < frameType - 251; ++i) {
                    locals.emplace_back(createVerificationTypeInfo(is));
                }
            }
        };

        struct FullFrame : public StackMapFrame {
            u2 offsetDelta{};
            u2 numberOfLocals{};
            std::vector<std::unique_ptr<VerificationTypeInfo>> locals;
            u2 numberOfStackItems{};
            std::vector<std::unique_ptr<VerificationTypeInfo>> stack;

            explicit FullFrame(std::istream &is) : StackMapFrame(is) {
                read(offsetDelta, is);
                read(numberOfLocals, is);
                if (numberOfLocals > 0) {
                    for (auto i = 0; i < numberOfLocals; ++i) {
                        locals.emplace_back(createVerificationTypeInfo(is));
                    }
                }
                read(numberOfStackItems, is);
                if (numberOfStackItems > 0) {
                    for (auto i = 0; i < numberOfStackItems; ++i) {
                        stack.emplace_back(createVerificationTypeInfo(is));
                    }
                }
            }
        };

        u2 numberOfEntries{};
        std::vector<std::unique_ptr<StackMapFrame>> entities;

        static std::unique_ptr<StackMapFrame> parseStackMapFrame(std::istream &is) {
            const StreamByteType auto frameType = peek<u1>(is);
            if (frameType >= 0 && frameType <= 63) {
                return std::make_unique<SameFrame>(is);
            } else if (frameType >= 64 && frameType <= 127) {
                return std::make_unique<SameLocals1StackItemFrame>(is);
            } else if (frameType == 247) {
                return std::make_unique<SameLocals1StackItemFrameExtended>(is);
            } else if (frameType >= 248 && frameType <= 250) {
                return std::make_unique<ChopFrame>(is);
            } else if (frameType == 251) {
                return std::make_unique<SameFrameExtended>(is);
            } else if (frameType >= 252 && frameType <= 254) {
                return std::make_unique<AppendFrame>(is);
            } else if (frameType == 255) {
                return std::make_unique<FullFrame>(is);
            } else {
                panic(cformat("parseStackMapFrame error {}", frameType));
            }
            //disable warning
            return nullptr;
        }

        explicit StackMapTableAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfEntries, is);
            if (numberOfEntries > 0) {
                for (auto i = 0; i < numberOfEntries; ++i) {
                    entities.emplace_back(parseStackMapFrame(is));
                }
            }
        }
    };

    struct CodeStub {
        std::vector<u1> stub;

        void push(u1 code) {
            stub.emplace_back(code);
        }

        void push(u2 code) {
            stub.emplace_back((code >> 8) & 0xFF);
            stub.emplace_back(code & 0xFF);
        }

        void push(const std::vector<u1> &codeStub) {
            stub.insert(stub.end(), codeStub.begin(), codeStub.end());
        }

    };

    struct Value : public CodeStub {
    };

    struct ElementValue : public CodeStub {
        u1 tag{};
        std::unique_ptr<Value> value;

        explicit ElementValue(std::istream &is);
    };

    struct ConstValue : public Value {
        u2 constValueIndex{};

        explicit ConstValue(std::istream &is) {
            read(constValueIndex, is);
            push(constValueIndex);
        }
    };

    struct EnumConstValue : public Value {
        u2 typeNameIndex{};
        u2 constNameIndex{};

        explicit EnumConstValue(std::istream &is) {
            read(typeNameIndex, is);
            read(constNameIndex, is);
            push(typeNameIndex);
            push(constNameIndex);
        }
    };

    struct ClassInfoValue : public Value {
        u2 classInfoIndex{};

        explicit ClassInfoValue(std::istream &is) {
            read(classInfoIndex, is);
            push(classInfoIndex);
        }
    };

    struct ArrayValue : public Value {
        u2 numberOfValues{};
        std::vector<std::unique_ptr<ElementValue>> values;

        explicit ArrayValue(std::istream &is) {
            read(numberOfValues, is);
            push(numberOfValues);
            for (auto i = 0; i < numberOfValues; ++i) {
                auto eleValue = std::make_unique<ElementValue>(is);
                push(eleValue->stub);
                values.emplace_back(std::move(eleValue));
            }
        }
    };

    struct Annotation : public Value {
        struct ElementValuePairs : public CodeStub {
            u2 elementNameIndex{};
            std::unique_ptr<ElementValue> value;

            explicit ElementValuePairs(std::istream &is) {
                read(elementNameIndex, is);
                push(elementNameIndex);

                value = std::make_unique<ElementValue>(is);
                push(value->stub);
            }
        };

        u2 typeIndex{};
        u2 numberOfElementValuePairs{};
        std::vector<std::unique_ptr<ElementValuePairs>> elementValuePairs;

        explicit Annotation(std::istream &is) {
            read(typeIndex, is);
            read(numberOfElementValuePairs, is);
            push(typeIndex);
            push(numberOfElementValuePairs);

            for (auto i = 0; i < numberOfElementValuePairs; ++i) {
                auto eleParis = std::make_unique<ElementValuePairs>(is);
                push(eleParis->stub);
                elementValuePairs.emplace_back(std::move(eleParis));
            }
        }
    };

    struct TypeAnnotation : public CodeStub {

        struct TargetInfo : public CodeStub {
        };

        struct TypeParameterTarget : public TargetInfo {
            u1 typeParameterIndex{};

            explicit TypeParameterTarget(std::istream &is) {
                read(typeParameterIndex, is);
                push(typeParameterIndex);
            }
        };

        struct SuperTypeTarget : public TargetInfo {
            u2 superTypeIndex{};

            explicit SuperTypeTarget(std::istream &is) {
                read(superTypeIndex, is);
                push(superTypeIndex);
            }
        };

        struct TypeParameterBoundTarget : public TargetInfo {
            u1 typeParameterIndex{};
            u1 boundIndex{};

            explicit TypeParameterBoundTarget(std::istream &is) {
                read(typeParameterIndex, is);
                read(boundIndex, is);
                push(typeParameterIndex);
                push(boundIndex);
            }
        };

        struct EmptyTarget : public TargetInfo {
            explicit EmptyTarget(std::istream &) {
            }
        };

        struct FormalParameterTarget : public TargetInfo {
            u1 formalParameterIndex{};

            explicit FormalParameterTarget(std::istream &is) {
                read(formalParameterIndex, is);
                push(formalParameterIndex);
            }
        };

        struct ThrowsTarget : public TargetInfo {
            u2 throwsTypeIndex{};

            explicit ThrowsTarget(std::istream &is) {
                read(throwsTypeIndex, is);
                push(throwsTypeIndex);
            }
        };

        struct LocalVarTarget : public TargetInfo {
            struct Table : public CodeStub {
                u2 startPC{};
                u2 length{};
                u2 index{};

                explicit Table(std::istream &is) {
                    read(startPC, is);
                    read(length, is);
                    read(index, is);
                    push(startPC);
                    push(length);
                    push(index);
                }
            };

            u2 tableLength{};
            std::vector<std::unique_ptr<Table>> tables;

            explicit LocalVarTarget(std::istream &is) {
                read(tableLength, is);
                push(tableLength);
                for (auto i = 0; i < tableLength; ++i) {
                    auto table = std::make_unique<Table>(is);
                    push(table->stub);
                    tables.emplace_back(std::move(table));
                }
            }
        };

        struct CatchTarget : public TargetInfo {
            u2 exceptionTableIndex{};

            explicit CatchTarget(std::istream &is) {
                read(exceptionTableIndex, is);
                push(exceptionTableIndex);
            }
        };

        struct OffsetTarget : public TargetInfo {
            u2 offset{};

            explicit OffsetTarget(std::istream &is) {
                read(offset, is);
                push(offset);
            }
        };

        struct TypeArgumentTarget : public TargetInfo {
            u2 offset{};
            u1 typeArgumentIndex{};

            explicit TypeArgumentTarget(std::istream &is) {
                read(offset, is);
                read(typeArgumentIndex, is);
                push(offset);
                push(typeArgumentIndex);
            }
        };

        struct TypePath : public CodeStub {
            struct Path : public CodeStub {
                u1 typePathKind{};
                u1 typeArgumentIndex{};

                explicit Path(std::istream &is) {
                    read(typePathKind, is);
                    read(typeArgumentIndex, is);
                    push(typePathKind);
                    push(typeArgumentIndex);
                }
            };

            u1 pathLength{};
            std::vector<std::unique_ptr<Path>> paths;

            explicit TypePath(std::istream &is) {
                read(pathLength, is);
                push(pathLength);
                for (auto i = 0; i < pathLength; ++i) {
                    auto path = std::make_unique<Path>(is);
                    push(path->stub);
                    paths.emplace_back(std::move(path));
                }
            }
        };

        u1 targetType{};
        std::unique_ptr<TargetInfo> targetInfo;
        std::unique_ptr<TypePath> targetPath;
        std::unique_ptr<Annotation> annotation;

        explicit TypeAnnotation(std::istream &is) {
            read(targetType, is);
            if (targetType == 0x00 || targetType == 0x01) {
                targetInfo = std::make_unique<TypeParameterTarget>(is);
            } else if (targetType == 0x10) {
                targetInfo = std::make_unique<SuperTypeTarget>(is);
            } else if (targetType == 0x11 || targetType == 0x12) {
                targetInfo = std::make_unique<TypeParameterBoundTarget>(is);
            } else if (targetType == 0x13 || targetType == 0x14 || targetType == 0x15) {
                targetInfo = std::make_unique<EmptyTarget>(is);
            } else if (targetType == 0x16) {
                targetInfo = std::make_unique<FormalParameterTarget>(is);
            } else if (targetType == 0x17) {
                targetInfo = std::make_unique<ThrowsTarget>(is);
            } else if (targetType == 0x40 || targetType == 0x41) {
                targetInfo = std::make_unique<LocalVarTarget>(is);
            } else if (targetType == 0x42) {
                targetInfo = std::make_unique<CatchTarget>(is);
            } else if (targetType == 0x43 || targetType == 0x44 || targetType == 0x45 || targetType == 0x46) {
                targetInfo = std::make_unique<OffsetTarget>(is);
            } else if (targetType == 0x47 || targetType == 0x48 || targetType == 0x49 || targetType == 0x4A ||
                       targetType == 0x4B) {
                targetInfo = std::make_unique<TypeArgumentTarget>(is);
            } else {
                panic(cformat("TypeAnnotation error targetType {}", targetType));
            }
            targetPath = std::make_unique<TypePath>(is);
            annotation = std::make_unique<Annotation>(is);
            push(targetType);
            push(targetInfo->stub);
            push(targetPath->stub);
            push(annotation->stub);
        }

    };

    struct ParameterAnnotations : public CodeStub {
        u2 numberOfAnnotations{};
        std::vector<std::unique_ptr<Annotation>> annotations;

        explicit ParameterAnnotations(std::istream &is) {
            read(numberOfAnnotations, is);
            push(numberOfAnnotations);
            for (auto i = 0; i < numberOfAnnotations; ++i) {
                auto annotation = std::make_unique<Annotation>(is);
                push(annotation->stub);
                annotations.emplace_back(std::move(annotation));
            }
        }
    };

    struct AnnotationAttribute : public AttributeInfo {
        std::unique_ptr<ParameterAnnotations> parameterAnnotations;

        explicit AnnotationAttribute(std::istream &is) : AttributeInfo(is) {
            parameterAnnotations = std::make_unique<ParameterAnnotations>(is);
        }
    };

    struct RuntimeVisibleAnnotationsAttribute : public AnnotationAttribute {
        explicit RuntimeVisibleAnnotationsAttribute(std::istream &is) : AnnotationAttribute(is) {
        }
    };

    struct RuntimeInvisibleAnnotationsAttribute : public AnnotationAttribute {
        explicit RuntimeInvisibleAnnotationsAttribute(std::istream &is) : AnnotationAttribute(is) {
        }
    };

    struct RuntimeVisibleParameterAnnotationsAttribute : public AttributeInfo {
        u1 numberOfParameters{};
        std::vector<std::unique_ptr<ParameterAnnotations>> parameterAnnotations;

        explicit RuntimeVisibleParameterAnnotationsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfParameters, is);
            for (auto i = 0; i < numberOfParameters; ++i) {
                auto parameterAnnotation = std::make_unique<ParameterAnnotations>(is);
                parameterAnnotations.emplace_back(std::move(parameterAnnotation));
            }
        }
    };

    struct RuntimeInvisibleParameterAnnotationsAttribute : public AttributeInfo {
        u1 numberOfParameters{};
        std::vector<std::unique_ptr<ParameterAnnotations>> parameterAnnotations;

        explicit RuntimeInvisibleParameterAnnotationsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfParameters, is);
            for (auto i = 0; i < numberOfParameters; ++i) {
                auto parameterAnnotation = std::make_unique<ParameterAnnotations>(is);
                parameterAnnotations.emplace_back(std::move(parameterAnnotation));
            }
        }
    };

    struct RuntimeVisibleTypeAnnotationsAttribute : public AttributeInfo {
        u1 numberOfAnnotation{};
        std::vector<std::unique_ptr<TypeAnnotation>> typeAnnotations;

        explicit RuntimeVisibleTypeAnnotationsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfAnnotation, is);
            for (auto i = 0; i < numberOfAnnotation; ++i) {
                auto annotation = std::make_unique<TypeAnnotation>(is);
                typeAnnotations.emplace_back(std::move(annotation));
            }
        }
    };

    struct RuntimeInvisibleTypeAnnotationsAttribute : public AttributeInfo {
        u1 numberOfAnnotation{};
        std::vector<std::unique_ptr<TypeAnnotation>> typeAnnotations;

        explicit RuntimeInvisibleTypeAnnotationsAttribute(std::istream &is) : AttributeInfo(is) {
            read(numberOfAnnotation, is);
            for (auto i = 0; i < numberOfAnnotation; ++i) {
                auto annotation = std::make_unique<TypeAnnotation>(is);
                typeAnnotations.emplace_back(std::move(annotation));
            }
        }
    };

    struct AnnotationDefaultAttribute : public AttributeInfo {
        std::unique_ptr<ElementValue> defaultValue;

        explicit AnnotationDefaultAttribute(std::istream &is) : AttributeInfo(is) {
            defaultValue = std::make_unique<ElementValue>(is);
        }
    };

    struct ByteStreamAttribute : public AttributeInfo {
        std::unique_ptr<u1[]> bytes;

        explicit ByteStreamAttribute(std::istream &is) : AttributeInfo(is) {
            if (attributeLength > 0) {
                bytes = readBuffer(is, attributeLength);
            }
        }
    };

}


#endif