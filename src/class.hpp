#ifndef CLASS_HPP
#define CLASS_HPP

#include "config.hpp"
#include "basic_type.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include "class_member.hpp"

namespace RexVM {

    struct ClassFile;
    struct ClassLoader;
    struct ConstantInfo;
    struct Oop;
    struct InstanceOop;
    struct MirrorOop;
    struct InstanceClass;
    struct Frame;

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

    struct Class {
        const ClassTypeEnum type;
        const u2 accessFlags{};
        const cstring name;
        std::unique_ptr<MirrorOop> mirror;
        ClassLoader &classLoader;
        ClassInitStatusEnum initStatus{ClassInitStatusEnum::Loaded};

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
        [[nodiscard]] bool isImplements(const Class *that) const;
        [[nodiscard]] bool isSubInterfaceOf(const Class *that) const;
        [[nodiscard]] bool isSuperClassOf(const Class *that) const;
        [[nodiscard]] bool isSuperInterfaceOf(const Class *that) const;
        [[nodiscard]] bool isSubClassOf(const Class *that) const;

        [[nodiscard]] MirrorOop *getMirrorOop() const;

        virtual ~Class();
    };

    struct InstanceClass : Class {
        u2 instanceSlotCount{};
        u2 staticSlotCount{};
        std::vector<std::unique_ptr<ConstantInfo>> constantPool;

        //TODO to array
        std::vector<std::unique_ptr<Field>> fields;
        std::vector<std::unique_ptr<Method>> methods;

        std::unique_ptr<Slot[]> staticData;

    private:
        void calcFieldSlotId();
        void initStaticField();

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

    struct ObjArrayClass;

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