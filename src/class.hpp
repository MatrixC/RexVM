#ifndef CLASS_HPP
#define CLASS_HPP

#include "config.hpp"
#include "basic_type.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <atomic>
#include "class_member.hpp"
#include "attribute_info.hpp"

namespace RexVM {

    struct ClassFile;
    struct ClassLoader;
    struct ConstantInfo;
    struct Oop;
    struct InstanceOop;
    struct MirrorOop;
    struct InstanceClass;
    struct Frame;
    struct OopManager;

    enum class ClassInitStatusEnum {
        Loaded,
        Init,
        Inited,
    };

    enum class ClassTypeEnum {
        PrimitiveClass,
        InstanceClass,
        TypeArrayClass,
        ObjArrayClass,
    };

    //标记在InstanceClass上, 用于在new指令上提升效率
    enum class SpecialInstanceClass {
        NONE,
        THREAD_CLASS,
        CLASS_LOADER_CLASS,
    };

    struct Class {
        const ClassTypeEnum type;
        const u2 accessFlags{};
        const cstring name;
        std::unique_ptr<MirrorOop> mirror;
        ClassLoader &classLoader;
        std::atomic<ClassInitStatusEnum> initStatus{ClassInitStatusEnum::Loaded};

        InstanceClass *superClass{nullptr};
        std::vector<InstanceClass *> interfaces;

        explicit Class(ClassTypeEnum type, u2 accessFlags, cstring name, ClassLoader &classLoader);

        [[nodiscard]] bool isInstanceClass() const;
        [[nodiscard]] bool isInterface() const;
        [[nodiscard]] bool isArray() const;
        [[nodiscard]] bool isJavaObjectClass() const;
        [[nodiscard]] bool isJavaCloneable() const;
        [[nodiscard]] bool isSerializable() const;

        [[nodiscard]] bool isAssignableFrom(const Class *that) const;
        [[nodiscard]] bool isInstanceOf(const Class *that) const;
        [[nodiscard]] bool isImplements(const Class *that) const;
        [[nodiscard]] bool isSubInterfaceOf(const Class *that) const;
        [[nodiscard]] bool isSuperClassOf(const Class *that) const;
        [[nodiscard]] bool isSuperInterfaceOf(const Class *that) const;
        [[nodiscard]] bool isSubClassOf(const Class *that) const;

        [[nodiscard]] MirrorOop *getMirrorOop() const;

        virtual ~Class();
    };

    //Hotspot对Primitive类型没有建立Class, 只有对应的MirrorOop
    //我觉得建立这个PrimitiveClass整体会更统一方便一点,少做一些特殊处理
    struct PrimitiveClass : Class {
        BasicType basicType;
        explicit PrimitiveClass(BasicType basicType, ClassLoader &classLoader);

        [[nodiscard]] BasicType getBasicType() const;
        [[nodiscard]] bool isWideType() const;
        [[nodiscard]] Slot getValueFromBoxingOop(InstanceOop *oop) const;
        [[nodiscard]] InstanceOop *getBoxingOopFromValue(Slot value, OopManager &oopManager) const;
    };

    struct InstanceClass : Class {
        bool anonymous{false};
        SpecialInstanceClass specialInstanceClass{SpecialInstanceClass::NONE};
        u2 instanceSlotCount{};
        u2 staticSlotCount{};
        std::vector<std::unique_ptr<ConstantInfo>> constantPool;

        std::vector<std::unique_ptr<Field>> fields;
        std::vector<std::unique_ptr<Method>> methods;
        std::unique_ptr<AttributeInfo> bootstrapMethodsAttr;
        std::unique_ptr<AttributeInfo> enclosingMethodAttr;
        std::unique_ptr<AttributeInfo> innerClassesAttr;
        [[nodiscard]] BootstrapMethodsAttribute *getBootstrapMethodAttr() const;
        [[nodiscard]] EnclosingMethodAttribute *getEnclosingMethodAttr() const;
        [[nodiscard]] InnerClassesAttribute *getInnerClassesAttr() const;

        size_t runtimeVisibleAnnotationLength{};
        std::unique_ptr<u1[]> runtimeVisibleAnnotation;

        size_t runtimeVisibleTypeAnnotationLength;
        std::unique_ptr<u1[]> runtimeVisibleTypeAnnotation;

        std::unique_ptr<Slot[]> staticData;
        //跟实例offset一致 查询其SlotType
        std::unique_ptr<SlotTypeEnum[]> instanceDataType;
        //跟类的static offset一致 查询其SlotType
        std::unique_ptr<SlotTypeEnum[]> staticDataType;
        cstring sourceFile{};
        cstring signature{};

    private:
        void calcFieldSlotId();
        void initStaticField();
        void initAttributes(ClassFile &cf);
        void initFields(ClassFile &cf);
        void initMethods(ClassFile &cf);
        void initInterfaceAndSuperClass(ClassFile &cf);
        void initSpecialType();
        void moveConstantPool(ClassFile &cf);

    public:

        explicit InstanceClass(ClassLoader &classLoader, ClassFile &cf);
        using Class::Class;

        ~InstanceClass() override = default;

        [[nodiscard]] bool notInitialize() const;
        void clinit(Frame &frame);

        [[nodiscard]] ClassMember *getMemberByRefIndex(size_t refIndex, ClassMemberTypeEnum type, bool isStatic) const;
        [[nodiscard]] Field *getField(const cstring &name, const cstring &descriptor, bool isStatic) const;
        [[nodiscard]] Field *getRefField(size_t refIndex, bool isStatic) const;
        [[nodiscard]] Method *getMethod(const cstring &name, const cstring &descriptor, bool isStatic) const;
        [[nodiscard]] Method *getRefMethod(size_t refIndex, bool isStatic) const;

        void setFieldValue(size_t index, Slot value) const;
        void setFieldValue(const cstring &name, const cstring &descriptor, Slot value) const;
        [[nodiscard]] Slot getFieldValue(size_t index) const;
        [[nodiscard]] Slot getFieldValue(const cstring &name, const cstring &descriptor) const;


    };

    struct ArrayClass : InstanceClass {
        size_t dimension{1};
        ArrayClass *higherDimension{};
        ArrayClass *lowerDimension{};

        explicit ArrayClass(ClassTypeEnum type, const cstring &name, ClassLoader &classLoader,
                            size_t dimension);

        ~ArrayClass() override = default;

        [[nodiscard]] cstring getComponentClassName() const;
    };

    struct TypeArrayClass : ArrayClass {
        const BasicType elementType;

        explicit TypeArrayClass(const cstring &name, ClassLoader &classLoader, size_t dimension,
                                BasicType elementType);

    };

    struct ObjArrayClass : ArrayClass {
        const InstanceClass *elementClass;

        explicit ObjArrayClass(const cstring &name, ClassLoader &classLoader, size_t dimension,
                               const InstanceClass *elementClass);

    };

}

#endif