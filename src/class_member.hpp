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


    struct ClassMember {
        const ClassMemberTypeEnum type;
        const u2 accessFlags{};
        const cstring name;
        const cstring descriptor;
        cstring signature;
        InstanceClass &klass;
        SlotTypeEnum slotType{SlotTypeEnum::NONE}; //returnSlotTyoe for method

        u2 slotId{};

        std::unique_ptr<BasicAnnotationContainer> basicAnnotationContainer;

        explicit ClassMember(ClassMemberTypeEnum type, u2 accessFlags, cstring name, cstring descriptor,
                             InstanceClass &klass);

        explicit ClassMember(ClassMemberTypeEnum type, InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf);

        [[nodiscard]] bool isStatic() const;
        [[nodiscard]] bool isFinal() const;
        [[nodiscard]] bool isPublic() const;
        [[nodiscard]] bool isPrivate() const;
        [[nodiscard]] bool isConstructor() const;
        [[nodiscard]] i4 getModifier() const;

        [[nodiscard]] bool is(const cstring &name, const cstring &descriptor) const;
        [[nodiscard]] bool is(const cstring &name, const cstring &descriptor, bool isStatic) const;

        MirrorBase mirrorBase{};
        [[nodiscard]] MirOop *getMirror(Frame *frame, bool init = true);

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
        u2 maxStack{};
        u2 maxLocals{};
        u4 codeLength{};
        std::unique_ptr<u1[]> code;
        std::vector<std::unique_ptr<ExceptionCatchItem>> exceptionCatches;
        std::vector<std::unique_ptr<LineNumberItem>> lineNumbers;
        std::vector<u2> exceptionsIndex;

        std::unique_ptr<MethodAnnotationContainer> methodAnnotationContainer;

        std::vector<cstring> paramType;
        cstring returnType;
        size_t paramSize{0};
        size_t paramSlotSize{0};
        std::vector<SlotTypeEnum> paramSlotType;

        [[nodiscard]] std::vector<Class *> getParamClasses() const;

        NativeMethodHandler nativeMethodHandler{};

        explicit Method(InstanceClass &klass, FMBaseInfo *info, const ClassFile &cf, u2 index);

        [[nodiscard]] bool isNative() const;
        [[nodiscard]] bool isAbstract() const;
        [[nodiscard]] bool isSynchronized() const;

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