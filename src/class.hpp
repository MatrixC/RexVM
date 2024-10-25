#ifndef CLASS_HPP
#define CLASS_HPP

#include "basic.hpp"
#include "basic_type.hpp"
#include <vector>
#include <memory>
#include <atomic>
#include "composite_string.hpp"
#include "utils/spin_lock.hpp"
#include "mirror_base.hpp"
#include "class_attribute_container.hpp"

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
    struct VMThread;
    struct MirOop;
    struct MirrorBase;
    struct BootstrapMethodsAttribute;
    struct EnclosingMethodAttribute;
    struct InnerClassesAttribute;
    struct ClassMember;
    struct Method;
    struct Field;
    struct AttributeInfo;

    struct Class {
        NameDescriptorIdentifier id;
        std::vector<InstanceClass *> interfaces;
        InstanceClass *superClass{nullptr};

        const ClassTypeEnum type;
        const u2 accessFlags{};
        
        ClassLoader &classLoader;
        std::atomic<ClassInitStatusEnum> initStatus{ClassInitStatusEnum::LOADED};

        //flags: low[accessFlags(16), type(2), anonymous(1), special(3), dimension, basicType(elementType) ]high

        explicit Class(ClassTypeEnum type, u2 accessFlags, cview name, ClassLoader &classLoader);

        void setName(cview name);
        [[nodiscard]] cview getClassName() const;
        [[nodiscard]] cview getClassDescriptor() const;
        [[nodiscard]] cview toView() const;
        [[nodiscard]] ClassTypeEnum getType() const;
        [[nodiscard]] u2 getAccessFlags() const;
        [[nodiscard]] size_t getInterfaceSize() const;
        [[nodiscard]] InstanceClass *getInterfaceByIndex(size_t index) const;
        [[nodiscard]] InstanceClass *getSuperClass() const;
        [[nodiscard]] SpecialClassEnum getSpecialClassType() const;

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
        
        MirrorBase mirrorBase{};
        [[nodiscard]] MirOop *getMirror(Frame *frame, bool init = true);

        
        ~Class();
    };

    //Hotspot对Primitive类型没有建立Class, 只有对应的MirrorOop
    //我觉得建立这个PrimitiveClass整体会更统一方便一点,少做一些特殊处理
    struct PrimitiveClass : Class {
        BasicType basicType;
        explicit PrimitiveClass(BasicType basicType, ClassLoader &classLoader);

        [[nodiscard]] BasicType getBasicType() const;
        [[nodiscard]] bool isWideType() const;
        [[nodiscard]] Slot getValueFromBoxingOop(InstanceOop *oop) const;
        [[nodiscard]] InstanceOop *getBoxingOopFromValue(Slot value, Frame &frame) const;
    };

    struct InstanceClass : Class {
        std::vector<std::unique_ptr<ConstantInfo>> constantPool;
        std::vector<std::unique_ptr<Field>> fields;
        std::vector<std::unique_ptr<Method>> methods;

        cview sourceFile{};
        MirrorBase constantPoolMirrorBase{};

        std::unique_ptr<BasicAnnotationContainer> basicAnnotationContainer;
        std::unique_ptr<ClassAttributeContainer> classAttributeContainer;
        std::unique_ptr<Slot[]> staticData;
        //跟实例offset一致 查询其SlotType
        std::unique_ptr<SlotTypeEnum[]> instanceDataType;
        //跟类的static offset一致 查询其SlotType
        std::unique_ptr<SlotTypeEnum[]> staticDataType;

        SpecialClassEnum specialClassType{SpecialClassEnum::NONE};
        u2 instanceSlotCount{};
        u2 staticSlotCount{};
        u2 signatureIndex{};
        bool overrideFinalize{false};

        [[nodiscard]] MirOop *getConstantPoolMirror(Frame *frame, bool init = true);
        [[nodiscard]] BootstrapMethodsAttribute *getBootstrapMethodAttr() const;
        [[nodiscard]] EnclosingMethodAttribute *getEnclosingMethodAttr() const;
        [[nodiscard]] InnerClassesAttribute *getInnerClassesAttr() const;

    private:
        void calcFieldSlotId();
        void initStaticField(VMThread &thread);
        void initAttributes(ClassFile &cf);
        void initFields(ClassFile &cf);
        void initMethods(ClassFile &cf);
        void initInterfaceAndSuperClass(ClassFile &cf);
        void initSpecialType();
        void moveConstantPool(ClassFile &cf);

    public:

        explicit InstanceClass(ClassLoader &classLoader, ClassFile &cf);
        using Class::Class;

        ~InstanceClass() = default;

        [[nodiscard]] bool notInitialize() const;
        void clinit(Frame &frame);

        [[nodiscard]] ClassMember *getMemberByRefIndex(size_t refIndex, ClassMemberTypeEnum type, bool isStatic) const;
        [[nodiscard]] Field *getFieldSelf(cview id, bool isStatic) const;
        [[nodiscard]] Field *getField(cview id, bool isStatic) const;
        [[nodiscard]] Field *getField(cview name, cview descriptor, bool isStatic) const;
        [[nodiscard]] Field *getRefField(size_t refIndex, bool isStatic) const;

        [[nodiscard]] Method *getMethodSelf(cview id, bool isStatic) const;
        [[nodiscard]] Method *getMethod(cview id, bool isStatic) const;
        [[nodiscard]] Method *getMethod(cview name, cview descriptor, bool isStatic) const;
        [[nodiscard]] Method *getRefMethod(size_t refIndex, bool isStatic) const;
        [[nodiscard]] cview getSignature() const;

        void setFieldValue(size_t index, Slot value) const;
        void setFieldValue(cview id, Slot value) const;
        [[nodiscard]] Slot getFieldValue(size_t index) const;
        [[nodiscard]] Slot getFieldValue(cview id) const;

    };

    struct ArrayClass : Class {
        size_t dimension{1};
        ArrayClass *higherDimension{};
        ArrayClass *lowerDimension{};

        explicit ArrayClass(ClassTypeEnum type, cview name, ClassLoader &classLoader, size_t dimension);

        ~ArrayClass() = default;

        [[nodiscard]] cview getComponentClassName() const;
    };

    struct TypeArrayClass : ArrayClass {
        const BasicType elementType;

        explicit TypeArrayClass(cview name, ClassLoader &classLoader, size_t dimension,
                                BasicType elementType);

    };

    struct ObjArrayClass : ArrayClass {
        const InstanceClass *elementClass;

        explicit ObjArrayClass(cview name, ClassLoader &classLoader, size_t dimension,
                               const InstanceClass *elementClass);

    };

}

#endif