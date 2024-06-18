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
                dynamic_cast<SignatureAttribute *>(info->getAssignAttribute(AttributeTagEnum::SIGNATURE)); 
            signatureAttribute != nullptr) {
            signature = getConstantStringFromPool(cf.constantPool, signatureAttribute->signatureIndex);
        }

        if (const auto runtimeVisibleAnnotationAttribute = 
                dynamic_cast<ByteStreamAttribute *>(info->getAssignAttribute(AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS));
            runtimeVisibleAnnotationAttribute != nullptr) {
            runtimeVisibleAnnotationLength = runtimeVisibleAnnotationAttribute->attributeLength;
            runtimeVisibleAnnotation = std::move(runtimeVisibleAnnotationAttribute->bytes);
        }
    }

    bool ClassMember::isStatic() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_STATIC)) != 0;
    }

    bool ClassMember::isFinal() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_FINAL)) != 0;
    }

    bool ClassMember::isPublic() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_PUBLIC)) != 0;
    }

    i4 ClassMember::getModifier() const {
        return (accessFlags & ~(static_cast<u2>(AccessFlagEnum::ACC_ANNOTATION)));
    }

    bool ClassMember::is(const cstring &name_, const cstring &descriptor_) const {
        return this->name == name_ && this->descriptor == descriptor_;
    }

    bool ClassMember::is(const cstring &name_, const cstring &descriptor_, bool isStatic) const {
        return is(name_, descriptor_) && this->isStatic() == isStatic;
    }

    ClassMember::~ClassMember() = default;

    Field::Field(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf) :
            ClassMember(ClassMemberTypeEnum::FIELD, klass, info, cf) {
        if (const auto constValueAttribute = 
                dynamic_cast<ConstantValueAttribute *>(info->getAssignAttribute(AttributeTagEnum::CONSTANT_VALUE)); 
            constValueAttribute != nullptr) {
            constantValueIndex = constValueAttribute->constantValueIndex;
        }
    }

    SlotTypeEnum Field::getFieldSlotType() const {
        auto const first = descriptor[0];
        return getSlotTypeByDescriptorFirstChar(first);
    }

    cstring Field::getTypeName() const {
        return getClassNameByFieldDescriptor(descriptor);
    }

    Class *Field::getTypeClass() const {
        return klass.classLoader.getClass(getTypeName());
    }

    bool Field::isWideType() {
        const auto first = descriptor[0];
        return isWideSlotType(first);
    }

    ExceptionCatchItem::ExceptionCatchItem(u2 start, u2 end, u2 handler, u2 catchType) :
        start(start), end(end), handler(handler), catchType(catchType) {
    }

    LineNumberItem::LineNumberItem(u2 start, u2 lineNumber) :
        start(start), lineNumber(lineNumber) {
    }

    Method::Method(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf) :
            ClassMember(ClassMemberTypeEnum::METHOD, klass, info, cf) {

        if (!isStatic()) {
            paramSize += 1;
            paramSlotSize += 1;
            paramSlotType.push_back(SlotTypeEnum::REF);
        }

        std::tie(paramDesc, returnTypeDesc) = parseMethodDescriptor(descriptor);
        paramSize = paramDesc.size();
        for (const auto &desc: paramDesc) {
            const auto first = desc[0];
            paramSlotSize += 1;
            const auto slotType = getSlotTypeByDescriptorFirstChar(first);

            paramSlotType.push_back(slotType);

            if (isWideSlotType(slotType)) {
               paramSlotSize += 1;  
               paramSlotType.push_back(slotType);
            }
        }

        if (const auto runtimeVisibleParameterAnnotationAttribute = 
                dynamic_cast<ByteStreamAttribute *>(info->getAssignAttribute(AttributeTagEnum::RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS));
            runtimeVisibleParameterAnnotationAttribute != nullptr) {
            runtimeVisibleParameterAnnotationLength = runtimeVisibleParameterAnnotationAttribute->attributeLength;
            runtimeVisibleParameterAnnotation = std::move(runtimeVisibleParameterAnnotationAttribute->bytes);
        }

        if (const auto runtimeVisibleTypeAnnotationAttribute = 
                dynamic_cast<ByteStreamAttribute *>(info->getAssignAttribute(AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS));
            runtimeVisibleTypeAnnotationAttribute != nullptr) {
            runtimeVisibleTypeAnnotationLength = runtimeVisibleTypeAnnotationAttribute->attributeLength;
            runtimeVisibleTypeAnnotation = std::move(runtimeVisibleTypeAnnotationAttribute->bytes);
        }


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

        if (const auto codeAttribute = dynamic_cast<CodeAttribute *>(info->getAssignAttribute(AttributeTagEnum::CODE));
            codeAttribute != nullptr) {
            maxStack = codeAttribute->maxStack;
            maxLocals = codeAttribute->maxLocals;
            codeLength = codeAttribute->codeLength;
            code = std::move(codeAttribute->code);
            if (codeAttribute->exceptionTableLength > 0) {
                exceptionCatches.reserve(codeAttribute->exceptionTableLength);
                for (const auto &exTableItem : codeAttribute->exceptionTables) {
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
                if (const auto lineNumberTableAttribute = dynamic_cast<LineNumberTableAttribute *>(
                    getAssignAttribute(
                        info->cf.constantPool,
                        codeAttribute->attributes,
                        AttributeTagEnum::LINE_NUMBER_TABLE
                    )); 
                    lineNumberTableAttribute != nullptr) {
                    lineNumbers.reserve(lineNumberTableAttribute->lineNumberTableLength);
                    for (const auto &attributeItem : lineNumberTableAttribute->lineNumberTables) {
                        lineNumbers.emplace_back(std::make_unique<LineNumberItem>(attributeItem->startPC, attributeItem->lineNumber));
                    }
                    lineNumberTableAttribute->lineNumberTables.clear();
                }
            }
        }
    }

    bool Method::isNative() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_NATIVE)) != 0;
    }

    bool Method::isAbstract() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_ABSTRACT)) != 0;
    }

    SlotTypeEnum Method::getParamSlotType(size_t slotIdx) const {
        return paramSlotType.at(slotIdx);
    }

    std::vector<Class *> Method::getParamClasses() const {
        std::vector<Class *> classes;
        for (const auto &desc: paramDesc) {
            const auto className = getDescriptorClassName(desc);
            classes.emplace_back(klass.classLoader.getClass(className));
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
                    return item->handler;
                }

                if (item->catchClass == nullptr) {
                    const auto &constantPool = klass.constantPool;
                    const auto classConstInfo = dynamic_cast<ConstantClassInfo *>(constantPool.at(
                            item->catchType).get());
                    const auto exClassName = getConstantStringFromPool(constantPool, classConstInfo->index);
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

    Method::~Method() = default;


}