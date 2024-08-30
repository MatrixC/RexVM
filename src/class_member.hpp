#ifndef CLASS_MEMBER_HPP
#define CLASS_MEMBER_HPP

#include "config.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
#include "utils/spin_lock.hpp"
#include "mirror_base.hpp"
#include "composite_ptr.hpp"
#include "class_annotation_container.hpp"

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct AttributeInfo;
    struct FMBaseInfo;
    struct ExceptionTable;
    struct LineNumberInfo;
    struct ClassFile;
    struct MirrorBase;

    using ClassMemberNameType = cstring;


    struct ClassMember {
        const ClassMemberNameType name__;
        const ClassMemberNameType descriptor__;

        cview getName() const;
        cview getDescriptor() const;

        // const rstring name;
        // const rstring descriptor;
        // cstring signature;

        InstanceClass &klass;
        MirrorBase mirrorBase{};
        std::unique_ptr<BasicAnnotationContainer> basicAnnotationContainer;

        const u2 accessFlags{};
        u2 slotId{};
        u2 signatureIndex{};

        const ClassMemberTypeEnum type;
        SlotTypeEnum slotType{SlotTypeEnum::NONE}; //returnSlotTyoe for method

        explicit ClassMember(ClassMemberTypeEnum type, u2 accessFlags, ClassMemberNameType name, ClassMemberNameType descriptor,
                             InstanceClass &klass);

        explicit ClassMember(ClassMemberTypeEnum type, InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf);

        [[nodiscard]] bool isStatic() const;
        [[nodiscard]] bool isFinal() const;
        [[nodiscard]] bool isPublic() const;
        [[nodiscard]] bool isPrivate() const;
        [[nodiscard]] i4 getModifier() const;
        [[nodiscard]] cstring getSignature() const;

        [[nodiscard]] bool is(const ClassMemberNameType &name, const ClassMemberNameType &descriptor) const;
        [[nodiscard]] bool is(const ClassMemberNameType &name, const ClassMemberNameType &descriptor, bool isStatic) const;

        [[nodiscard]] bool isConstructor() const;
        [[nodiscard]] bool isClInit() const;
        [[nodiscard]] bool isFinalize() const;

        [[nodiscard]] cview toView() const;

        [[nodiscard]] MirOop *getMirror(Frame *frame, bool init = true);

        static bool compareClassMemberName(const ClassMember *a, const ClassMember *b);

        ~ClassMember();
    };

    struct Field : ClassMember {
        u2 constantValueIndex{};
 
        explicit Field(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf);

        [[nodiscard]] SlotTypeEnum getFieldSlotType();
        [[nodiscard]] cstring getTypeName() const;
        [[nodiscard]] Class *getTypeClass() const;

        //Long or Double
        [[nodiscard]] bool isWideType() const;
    };

    struct ExceptionCatchItem {
        InstanceClass *catchClass{};
        const u2 start;
        const u2 end;
        const u2 handler;
        const u2 catchType;

        explicit ExceptionCatchItem(u2 start, u2 end, u2 handler, u2 catchType);
    };

    struct LineNumberItem {
        const u2 start;
        const u2 lineNumber;

        explicit LineNumberItem(u2 start, u2 lineNumber);
    };

    struct Method : ClassMember {
        u2 maxLocals{};
        u4 codeLength{};
        std::unique_ptr<u1[]> code;
        std::vector<std::unique_ptr<ExceptionCatchItem>> exceptionCatches;
        std::vector<std::unique_ptr<LineNumberItem>> lineNumbers;

        CompositeArray<u2> exceptionsIndex;

        std::unique_ptr<MethodAnnotationContainer> methodAnnotationContainer;

        std::vector<cstring> paramType;
        cstring returnType;
        size_t paramSlotSize{0};
        std::vector<SlotTypeEnum> paramSlotType;
        NativeMethodHandler nativeMethodHandler{};


        explicit Method(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf, u2 index);

        [[nodiscard]] bool isNative() const;
        [[nodiscard]] bool isAbstract() const;
        [[nodiscard]] bool isSynchronized() const;

        [[nodiscard]] std::vector<Class *> getParamClasses() const;
        [[nodiscard]] SlotTypeEnum getParamSlotType(size_t slotIdx) const;

        std::optional<i4> findExceptionHandler(const InstanceClass *exClass, u4 pc);
        [[nodiscard]] u4 getLineNumber(u4 pc) const;

        ~Method();

        private:
        void initParamSlotSize();
        void initAnnotations(FMBaseInfo *info);
        void initCode(FMBaseInfo *info);
        void initExceptions(FMBaseInfo *info);


    };


}

#endif