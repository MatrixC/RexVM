#include "class_member.hpp"

#include <utility>
#include <optional>
#include "constant_info.hpp"
#include "attribute_info.hpp"
#include "class.hpp"
#include "class_file.hpp"
#include "class_loader.hpp"
#include "utils/descriptor_parser.hpp"
#include "utils/class_utils.hpp"
#include "native/native_manager.hpp"

namespace RexVM {

    ClassMember::ClassMember(ClassMemberTypeEnum type, u2 accessFlags, cstring name, cstring descriptor,
                             InstanceClass &klass) :
            type(type), accessFlags(accessFlags), name(std::move(name)), descriptor(std::move(descriptor)),
            klass(klass) {
    }

    ClassMember::ClassMember(ClassMemberTypeEnum type, InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf) :
            ClassMember(type, info->accessFlags, info->getName(), info->getDescriptor(), klass) {

        if (const auto signatureAttribute =
                    CAST_SIGNATURE_ATTRIBUTE(info->getAssignAttribute(AttributeTagEnum::SIGNATURE));
                signatureAttribute != nullptr) {
            signature = getConstantStringFromPool(cf.constantPool, signatureAttribute->signatureIndex);
        }

        if (const auto runtimeVisibleAnnotationAttribute =
                    CAST_BYTE_STREAM_ATTRIBUTE(info->getAssignAttribute(
                            AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS));
                runtimeVisibleAnnotationAttribute != nullptr) {
            runtimeVisibleAnnotationLength = runtimeVisibleAnnotationAttribute->attributeLength;
            runtimeVisibleAnnotation = std::move(runtimeVisibleAnnotationAttribute->bytes);
        }

        if (const auto runtimeVisibleTypeAnnotationAttribute =
                    CAST_BYTE_STREAM_ATTRIBUTE(info->getAssignAttribute(
                            AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS));
                runtimeVisibleTypeAnnotationAttribute != nullptr) {
            runtimeVisibleTypeAnnotationLength = runtimeVisibleTypeAnnotationAttribute->attributeLength;
            runtimeVisibleTypeAnnotation = std::move(runtimeVisibleTypeAnnotationAttribute->bytes);
        }
    }

    bool ClassMember::isStatic() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_STATIC)) != 0;
    }

    bool ClassMember::isFinal() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_FINAL)) != 0;
    }

    bool ClassMember::isPublic() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_PUBLIC)) != 0;
    }

    bool ClassMember::isPrivate() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_PRIVATE)) != 0;
    }

    bool ClassMember::isConstructor() const {
        return type == ClassMemberTypeEnum::METHOD && name == "<init>";
    }

    i4 ClassMember::getModifier() const {
        return (accessFlags & ~(CAST_U2(AccessFlagEnum::ACC_ANNOTATION)));
    }

    bool ClassMember::is(const cstring &name_, const cstring &descriptor_) const {
        return this->name == name_ && this->descriptor == descriptor_;
    }

    bool ClassMember::is(const cstring &name_, const cstring &descriptor_, bool isStatic) const {
        return is(name_, descriptor_) && this->isStatic() == isStatic;
    }

    MirOop *ClassMember::getMirror(Frame *frame, bool init) {
        static SpinLock fieldLock;
        static SpinLock methodLock;

        SpinLock &lock = type == ClassMemberTypeEnum::FIELD ? fieldLock : methodLock;

        const auto mirrorType = 
            type == ClassMemberTypeEnum::FIELD ? MirrorObjectTypeEnum::FIELD
            : (isConstructor() ? MirrorObjectTypeEnum::CONSTRUCTOR : MirrorObjectTypeEnum::METHOD);

        return mirrorBase.getBaseMirror(frame, mirrorType, this, lock, init);
    }

    ClassMember::~ClassMember() = default;

    Field::Field(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf) :
            ClassMember(ClassMemberTypeEnum::FIELD, klass, info, cf) {
        if (const auto constValueAttribute =
                    CAST_CONSTANT_VALUE_ATTRIBUTE(info->getAssignAttribute(AttributeTagEnum::CONSTANT_VALUE));
                constValueAttribute != nullptr) {
            constantValueIndex = constValueAttribute->constantValueIndex;
        }
    }

    SlotTypeEnum Field::getFieldSlotType() {
        if (slotType != SlotTypeEnum::NONE) {
            return slotType;
        }
        auto const first = descriptor[0];
        slotType = getSlotTypeByDescriptorFirstChar(first);
        return slotType;
    }

    cstring Field::getTypeName() const {
        return getClassNameByFieldDescriptor(descriptor);
    }

    Class *Field::getTypeClass() const {
        return klass.classLoader.getClass(getTypeName());
    }

    bool Field::isWideType() const {
        return isWideSlotType(slotType);
    }

    ExceptionCatchItem::ExceptionCatchItem(u2 start, u2 end, u2 handler, u2 catchType) :
            start(start), end(end), handler(handler), catchType(catchType) {
    }

    LineNumberItem::LineNumberItem(u2 start, u2 lineNumber) :
            start(start), lineNumber(lineNumber) {
    }

    Method::Method(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf, u2 index) :
            ClassMember(ClassMemberTypeEnum::METHOD, klass, info, cf), index(index) {
        
        initParamSlotSize();
        initAnnotations(info);
        initCode(info);
        initExceptions(info);
    }

    void Method::initParamSlotSize() {
        if (!isStatic()) {
            paramSize += 1;
            paramSlotSize += 1;
            paramSlotType.emplace_back(SlotTypeEnum::REF);
        }
        std::tie(paramType, returnType) = parseMethodDescriptor(descriptor);
        paramSize += paramType.size();
        for (const auto &desc: paramType) {
            paramSlotSize += 1;

            SlotTypeEnum slotType = getSlotTypeByPrimitiveClassName(desc);
            paramSlotType.emplace_back(slotType);

            if (isWideSlotType(slotType)) {
                paramSlotSize += 1;
                paramSlotType.push_back(slotType);
            }
        }
        returnSlotType = getSlotTypeByPrimitiveClassName(returnType);
    }

    void Method::initAnnotations(FMBaseInfo *info) {
        if (const auto runtimeVisibleParameterAnnotationAttribute =
                    CAST_BYTE_STREAM_ATTRIBUTE(info->getAssignAttribute(
                            AttributeTagEnum::RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS));
                runtimeVisibleParameterAnnotationAttribute != nullptr) {
            runtimeVisibleParameterAnnotationLength = runtimeVisibleParameterAnnotationAttribute->attributeLength;
            runtimeVisibleParameterAnnotation = std::move(runtimeVisibleParameterAnnotationAttribute->bytes);
        }

        if (const auto annotationDefaultAttribute =
                    CAST_BYTE_STREAM_ATTRIBUTE(info->getAssignAttribute(
                            AttributeTagEnum::ANNOTATION_DEFAULT));
                annotationDefaultAttribute != nullptr) {
            annotationDefaultLength = annotationDefaultAttribute->attributeLength;
            annotationDefault = std::move(annotationDefaultAttribute->bytes);
        }
    }

    void Method::initCode(FMBaseInfo *info) {
        if (isNative()) {
            nativeMethodHandler =
                    NativeManager::instance.getNativeMethod(
                            klass.name,
                            name,
                            descriptor,
                            isStatic()
                    );
            return;
        }

        if (const auto codeAttribute = CAST_CODE_ATTRIBUTE(info->getAssignAttribute(AttributeTagEnum::CODE));
                codeAttribute != nullptr) {
            maxStack = codeAttribute->maxStack;
            maxLocals = codeAttribute->maxLocals;
            codeLength = codeAttribute->codeLength;
            code = std::move(codeAttribute->code);
            if (codeAttribute->exceptionTableLength > 0) {
                exceptionCatches.reserve(codeAttribute->exceptionTableLength);
                for (const auto &exTableItem: codeAttribute->exceptionTables) {
                    auto catchItem = std::make_unique<ExceptionCatchItem>(
                            exTableItem->startPC,
                            exTableItem->endPC,
                            exTableItem->handlerPC,
                            exTableItem->catchType
                    );
                    exceptionCatches.emplace_back(std::move(catchItem));
                }
                codeAttribute->exceptionTables.clear();
            }

            if (!codeAttribute->attributes.empty()) {
                if (const auto lineNumberTableAttribute = CAST_LINE_NUMBER_ATTRIBUTE(
                            getAssignAttributeByConstantPool(
                                    info->cf.constantPool,
                                    codeAttribute->attributes,
                                    AttributeTagEnum::LINE_NUMBER_TABLE
                            ));
                        lineNumberTableAttribute != nullptr) {

                    if (!lineNumberTableAttribute->lineNumberTables.empty()) {
                        lineNumbers.reserve(lineNumberTableAttribute->lineNumberTables.size());
                        for (const auto &attributeItem: lineNumberTableAttribute->lineNumberTables) {
                            lineNumbers.emplace_back(
                                    std::make_unique<LineNumberItem>(attributeItem->startPC, attributeItem->lineNumber));
                        }
                    }
                    lineNumberTableAttribute->lineNumberTables.clear();
                }
            }
        }
    }

    void Method::initExceptions(FMBaseInfo *info) {
        const auto exceptionsAttribute = CAST_EXCEPTIONS_ATTRIBUTE(info->getAssignAttribute(AttributeTagEnum::EXCEPTIONS));
        if (exceptionsAttribute != nullptr) {
            exceptionsIndex.reserve(exceptionsAttribute->numberOfExceptions);
            exceptionsIndex.assign(exceptionsAttribute->exceptionIndexTable.begin(), exceptionsAttribute->exceptionIndexTable.end());
        }
    }

    bool Method::isNative() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_NATIVE)) != 0;
    }

    bool Method::isAbstract() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_ABSTRACT)) != 0;
    }

    bool Method::isSynchronized() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_SYNCHRONIZED)) != 0;
    }

    SlotTypeEnum Method::getParamSlotType(size_t slotIdx) const {
        return paramSlotType[slotIdx];
    }

    std::vector<Class *> Method::getParamClasses() const {
        std::vector<Class *> classes;
        classes.reserve(paramType.size());
        for (const auto &desc: paramType) {
            classes.emplace_back(klass.classLoader.getClass(desc));
        }
        return classes;
    }

    std::optional<i4> Method::findExceptionHandler(const InstanceClass *exClass, u4 pc) {
        if (exceptionCatches.empty()) {
            return std::nullopt;
        }

        for (const auto &item: exceptionCatches) {
            if (pc >= item->start && pc < item->end) {
                if (item->catchType == 0) {
                    //catch (Exception ex) { xx }
                    return item->handler;
                }

                if (item->catchClass == nullptr) {
                    const auto &constantPool = klass.constantPool;
                    const auto exClassName = getConstantStringFromPoolByIndexInfo(constantPool, item->catchType);
                    if (exClass->name == exClassName) {
                        //Optimize[catchClass == exClass], needn't load Exception Class
                        return item->handler;
                    }
                    item->catchClass = klass.classLoader.getInstanceClass(exClassName);
                }

                const auto catchClass = item->catchClass;
                if (catchClass == exClass || catchClass->isSuperClassOf(exClass)) {
                    return item->handler;
                }
            }
        }

        return std::nullopt;
    }

    u4 Method::getLineNumber(u4 pc) const {
        if (isNative() || lineNumbers.empty()) {
            return 0;
        }

        for (i4 i = CAST_I4(lineNumbers.size()) - 1; i >= 0; --i) {
            const auto &item = lineNumbers[i];
            if (pc > item->start) {
                return item->lineNumber;
            }
        }

        return 0;
    }

    Method::~Method() = default;


}