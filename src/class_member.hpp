#ifndef CLASS_MEMBER_HPP
#define CLASS_MEMBER_HPP

#include "config.hpp"
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct AttributeInfo;
    struct FMBaseInfo;
    struct ExceptionTable;
    struct LineNumberInfo;
    struct ClassFile;

    enum class ClassMemberTypeEnum {
        FIELD,
        METHOD,
    };

    struct ClassMember {
        const ClassMemberTypeEnum type;
        const u2 accessFlags{};
        const cstring name;
        const cstring descriptor;
        cstring signature;
        InstanceClass &klass;

        size_t runtimeVisibleAnnotationLength{};
        std::unique_ptr<u1[]> runtimeVisibleAnnotation;

        size_t runtimeVisibleTypeAnnotationLength{};
        std::unique_ptr<u1[]> runtimeVisibleTypeAnnotation;

        explicit ClassMember(ClassMemberTypeEnum type, u2 accessFlags, cstring name, cstring descriptor,
                             InstanceClass &klass);

        explicit ClassMember(ClassMemberTypeEnum type, InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf);

        [[nodiscard]] bool isStatic() const;
        [[nodiscard]] bool isFinal() const;
        [[nodiscard]] bool isPublic() const;
        [[nodiscard]] bool isPrivate() const;
        [[nodiscard]] i4 getModifier() const;

        [[nodiscard]] bool is(const cstring &name, const cstring &descriptor) const;
        [[nodiscard]] bool is(const cstring &name, const cstring &descriptor, bool isStatic) const;

        virtual ~ClassMember();
    };

    struct Field : ClassMember {
        u2 constantValueIndex{};
        u2 slotId{};
        SlotTypeEnum slotType{SlotTypeEnum::NONE};
 
        explicit Field(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf);

        [[nodiscard]] SlotTypeEnum getFieldSlotType();

        [[nodiscard]] cstring getTypeName() const;
        [[nodiscard]] Class *getTypeClass() const;

        //Long or Double
        [[nodiscard]] bool isWideType() const;
    };

    struct ExceptionCatchItem {
        const u2 start;
        const u2 end;
        const u2 handler;
        const u2 catchType;
        InstanceClass *catchClass{};

        explicit ExceptionCatchItem(u2 start, u2 end, u2 handler, u2 catchType);
    };

    struct LineNumberItem {
        const u2 start;
        const u2 lineNumber;

        explicit LineNumberItem(u2 start, u2 lineNumber);
    };

    struct Method : ClassMember {
        u2 index{};
        u2 maxStack{};
        u2 maxLocals{};
        u4 codeLength{};
        std::unique_ptr<u1[]> code;
        std::vector<std::unique_ptr<ExceptionCatchItem>> exceptionCatches;
        std::vector<std::unique_ptr<LineNumberItem>> lineNumbers;
        std::vector<u2> exceptionsIndex;

        size_t runtimeVisibleParameterAnnotationLength{};
        std::unique_ptr<u1[]> runtimeVisibleParameterAnnotation;

        size_t annotationDefaultLength{};
        std::unique_ptr<u1[]> annotationDefault;

        std::vector<cstring> paramType;
        cstring returnType;
        size_t paramSize{0};
        size_t paramSlotSize{0};
        std::vector<SlotTypeEnum> paramSlotType;
        SlotTypeEnum returnSlotType{SlotTypeEnum::NONE};

        [[nodiscard]] std::vector<Class *> getParamClasses() const;

        NativeMethodHandler nativeMethodHandler{};

        explicit Method(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf, u2 index);

        [[nodiscard]] bool isNative() const;
        [[nodiscard]] bool isAbstract() const;
        [[nodiscard]] bool isSynchronized() const;

        [[nodiscard]] SlotTypeEnum getParamSlotType(size_t slotIdx) const;

        std::optional<i4> findExceptionHandler(const InstanceClass *exClass, u4 pc);
        [[nodiscard]] u4 getLineNumber(u4 pc) const;

        ~Method() override;


        private:
        void initParamSlotSize();
        void initAnnotations(FMBaseInfo *info);
        void initCode(FMBaseInfo *info);
        void initExceptions(FMBaseInfo *info);


    };


}

#endif