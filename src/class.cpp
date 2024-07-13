#include "class.hpp"

#include <utility>
#include "basic_java_class.hpp"
#include "constant_pool.hpp"
#include "class_member.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "vm.hpp"
#include "thread.hpp"
#include "frame.hpp"
#include "exception.hpp"
#include "memory.hpp"

namespace RexVM {

    Class::Class(const ClassTypeEnum type, const u2 accessFlags, cstring name, ClassLoader &classLoader) :
            type(type), accessFlags(accessFlags), name(std::move(name)), classLoader(classLoader) {
    }

    bool Class::isInstanceClass() const {
        return type == ClassTypeEnum::InstanceClass;
    }

    bool Class::isInterface() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_INTERFACE)) != 0;
    }

    bool Class::isArray() const {
        return type == ClassTypeEnum::TypeArrayClass || type == ClassTypeEnum::ObjArrayClass;
    }

    bool Class::isJavaObjectClass() const {
        return name == JAVA_LANG_OBJECT_NAME;
    }

    bool Class::isJavaCloneable() const {
        return name == JAVA_LANG_CLONEABLE_NAME;
    }

    bool Class::isSerializable() const {
        return name == JAVA_IO_SERIALIZABLE_NAME;
    }

    bool Class::isAssignableFrom(const Class *that) const {
        auto s = that;
        auto t = this;

        if (s == t) {
            return true;
        }

        if (!s->isArray()) {
            if (!s->isInterface()) {
                if (!t->isInterface()) {
                    return s->isSubClassOf(t);
                } else {
                    return s->isImplements(t);
                }
            } else {
                if (!t->isInterface()) {
                    return t->isJavaObjectClass();
                } else {
                    return t->isSuperInterfaceOf(s);
                }
            }
        } else {
            if (!t->isArray()) {
                if (!t->isInterface()) {
                    return t->isJavaObjectClass();
                } else {
                    return t->isJavaCloneable() || t->isSerializable();
                }
            } else {
                const auto sName = (static_cast<const ArrayClass *>(s))->getComponentClassName();
                const auto tName = (static_cast<const ArrayClass *>(s))->getComponentClassName();
                return sName == tName ||
                       classLoader.getClass(tName)->isAssignableFrom(classLoader.getClass(sName));
            }
        }
    }

    bool Class::isSubClassOf(const Class *that) const {
        for (auto c = this->superClass; c != nullptr; c = c->superClass) {
            if (c == that) {
                return true;
            }
        }
        return false;
    }

    bool Class::isImplements(const Class *that) const {
        if (!isInstanceClass() || !that->isInstanceClass()) {
            return false;
        }
        auto self = static_cast<const InstanceClass *>(this);
        auto interface = static_cast<const InstanceClass *>(that);
        for (auto c = self; c != nullptr; c = c->superClass) {
            for (const auto &item: c->interfaces) {
                if (item == interface || item->isSubInterfaceOf(interface)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Class::isSubInterfaceOf(const Class *that) const {
        if (!isInstanceClass() || !that->isInstanceClass()) {
            return false;
        }
        auto self = static_cast<const InstanceClass *>(this);
        auto interface = static_cast<const InstanceClass *>(that);
        for (const auto &item: self->interfaces) {
            if (item == interface || item->isSubInterfaceOf(interface)) {
                return true;
            }
        }

        return false;

    }

    bool Class::isSuperClassOf(const Class *that) const {
        return that->isSubClassOf(this);
    }

    bool Class::isSuperInterfaceOf(const Class *that) const {
        return that->isSubInterfaceOf(this);
    }

    constexpr auto PRIMITIVE_CLASS_ACCESS_FLAGS = 
        CAST_U2(AccessFlagEnum::ACC_PUBLIC) |
        CAST_U2(AccessFlagEnum::ACC_FINAL) |
        CAST_U2(AccessFlagEnum::ACC_ABSTRACT);

    PrimitiveClass::PrimitiveClass(BasicType basicType, ClassLoader &classLoader) :
        Class(
            ClassTypeEnum::PrimitiveClass, 
            PRIMITIVE_CLASS_ACCESS_FLAGS, 
            basicTypeClassName(basicType), 
            classLoader
        ),
        basicType(basicType) {
    }

    BasicType PrimitiveClass::getBasicType() const {
        return basicType;
    }

    bool PrimitiveClass::isWideType() const {
        return isWideBasicType(basicType);
    }

    Slot PrimitiveClass::getValueFromBoxingOop(InstanceOop *oop) const {
        switch (basicType) {
            case BasicType::T_BOOLEAN: return oop->getFieldValue("value", "Z");
            case BasicType::T_CHAR: return oop->getFieldValue("value", "C");
            case BasicType::T_FLOAT: return oop->getFieldValue("value", "F");
            case BasicType::T_DOUBLE: return oop->getFieldValue("value", "D");
            case BasicType::T_BYTE: return oop->getFieldValue("value", "B");
            case BasicType::T_SHORT: return oop->getFieldValue("value", "S");
            case BasicType::T_INT: return oop->getFieldValue("value", "I");
            case BasicType::T_LONG: return oop->getFieldValue("value", "J");
            case BasicType::T_VOID: panic("error type void");
            default:
                panic("error basicType");
                return Slot(0);
        }
    }

    InstanceOop *PrimitiveClass::getBoxingOopFromValue(Slot value, OopManager &oopManager) const {
        switch (basicType) {
            case BasicType::T_BOOLEAN: return oopManager.newBooleanOop(value.i4Val);
            case BasicType::T_CHAR: return oopManager.newCharOop(value.i4Val);
            case BasicType::T_FLOAT: return oopManager.newFloatOop(value.f4Val);
            case BasicType::T_DOUBLE: return oopManager.newDoubleOop(value.f8Val);
            case BasicType::T_BYTE: return oopManager.newByteOop(value.i4Val);
            case BasicType::T_SHORT: return oopManager.newShortOop(value.i4Val);
            case BasicType::T_INT: return oopManager.newIntegerOop(value.i4Val);
            case BasicType::T_LONG: return oopManager.newLongOop(value.i4Val);
            case BasicType::T_VOID: panic("error type void");
            default:
                panic("error basicType");
                return nullptr;
        }
    }

    MirrorOop * Class::getMirrorOop() const {
        return mirror.get();
    }

    Class::~Class() = default;

    InstanceClass::InstanceClass(ClassLoader &classLoader, ClassFile &cf) :
            Class(ClassTypeEnum::InstanceClass, cf.accessFlags, cf.getThisClassName(), classLoader) {

        sourceFile = cf.getSourceFile();
        signature = cf.getSignature();
        initAttributes(cf);
        initFields(cf);
        initMethods(cf);
        initInterfaceAndSuperClass(cf);
        moveConstantPool(cf);
        calcFieldSlotId();
        initStaticField();


    }

    void InstanceClass::initAttributes(ClassFile &cf) {
        for (auto &attribute: cf.attributes) {
            const auto attributeName = getConstantStringFromPool(cf.constantPool, (const size_t) attribute->attributeNameIndex);
            const auto attributeTypeEnum = ATTRIBUTE_NAME_TAG_MAP.at(attributeName);
            switch (attributeTypeEnum) {
                case AttributeTagEnum::BOOTSTRAP_METHODS:
                    bootstrapMethodsAttr = std::move(attribute);
                break;

                case AttributeTagEnum::ENCLOSING_METHOD:
                    enclosingMethodAttr = std::move(attribute);
                break;

                case AttributeTagEnum::INNER_CLASSES:
                    innerClassesAttr = std::move(attribute);
                break;

                case AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS: {
                    const auto runtimeVisibleAnnotationAttribute = CAST_BYTE_STREAM_ATTRIBUTE(attribute.get());
                    runtimeVisibleAnnotationLength = runtimeVisibleAnnotationAttribute->attributeLength;
                    runtimeVisibleAnnotation = std::move(runtimeVisibleAnnotationAttribute->bytes); 
                    break;
                }

                case AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS: {
                    const auto runtimeVisibleTypeAnnotationAttribute = CAST_BYTE_STREAM_ATTRIBUTE(attribute.get());
                    runtimeVisibleTypeAnnotationLength = runtimeVisibleTypeAnnotationAttribute->attributeLength;
                    runtimeVisibleTypeAnnotation = std::move(runtimeVisibleTypeAnnotationAttribute->bytes);
                    break;
                }

                default:
                    break;
            }
        }
    }

    void InstanceClass::initFields(ClassFile &cf) {
        fields.reserve(cf.fieldCount);
        for (const auto &fieldInfo: cf.fields) {
            auto field = 
                std::make_unique<Field>(
                    *this, 
                    fieldInfo.get(), 
                    cf
                );
            fields.emplace_back(std::move(field));
        }
    }

    void InstanceClass::initMethods(ClassFile &cf) {
        methods.reserve(cf.methodCount);
        u2 index = 0;
        for (const auto &methodInfo: cf.methods) {
            auto method =
                    std::make_unique<Method>(
                            *this,
                            methodInfo.get(),
                            cf,
                            index++
                    );
            methods.emplace_back(std::move(method));
        }
    }

    void InstanceClass::initInterfaceAndSuperClass(ClassFile &cf) {
        interfaces.reserve(cf.interfaceCount);
        for (const auto &interfaceName: cf.getInterfaceNames()) {
            interfaces.emplace_back(classLoader.getInstanceClass(interfaceName));
        }

        if (!isInterface()) {
            //OpenJDK实现, Interface没有SuperClass
            superClass = classLoader.getInstanceClass(cf.getSuperClassName());
        }
    }

    void InstanceClass::moveConstantPool(ClassFile &cf) {
        constantPool.reserve(cf.constantPool.size());
        std::move(cf.constantPool.begin(), cf.constantPool.end(), std::back_inserter(constantPool));
    }

    void InstanceClass::calcFieldSlotId() {
        u2 slotId{0};
        u2 staticSlotId{0};

        if (superClass != nullptr) {
            slotId = superClass->instanceSlotCount;
        }

        for (const auto &field: fields) {
            if (field->isStatic()) {
                field->slotId = staticSlotId;
                staticSlotId++;
            } else {
                field->slotId = slotId;
                slotId++;
            }
        }
        instanceSlotCount = slotId;
        staticSlotCount = staticSlotId;
    }

    void InstanceClass::initStaticField() {
        if (staticSlotCount <= 0) {
            return;
        }
        staticData = std::make_unique<Slot[]>(staticSlotCount);
        for (const auto &field: fields) {
            if (!field->isStatic()) {
                continue;
            }
            if (field->isFinal() && field->constantValueIndex > 0) {
                const auto &constValue = constantPool.at(field->constantValueIndex);
                const auto descriptor = field->descriptor;
                Slot data;
                if (descriptor == "Z" || descriptor == "B" || descriptor == "C" ||
                    descriptor == "S" || descriptor == "I") {
                    data = Slot(CAST_CONSTANT_INTEGER_INFO(constValue.get())->value);
                } else if (descriptor == "J") {
                    data = Slot(CAST_CONSTANT_LONG_INFO(constValue.get())->value);
                } else if (descriptor == "F") {
                    data = Slot(CAST_CONSTANT_FLOAT_INFO(constValue.get())->value);
                } else if (descriptor == "D") {
                    data = Slot(CAST_CONSTANT_DOUBLE_INFO(constValue.get())->value);
                } else if (descriptor == "Ljava/lang/String;") {
                    auto strOop = classLoader.vm.stringPool->getInternString(
                        getConstantStringFromPoolByIndexInfo(constantPool, field->constantValueIndex)
                    );
                    data = Slot(strOop);
                }
                staticData[field->slotId] = data;
            } else {
                const auto slotType = field->getFieldSlotType();
                switch (slotType) {
                    case SlotTypeEnum::REF:
                        staticData[field->slotId] = Slot(nullptr);
                        break;

                    case SlotTypeEnum::F4:
                        staticData[field->slotId] = Slot(CAST_F4(0));
                        break;

                    case SlotTypeEnum::F8:
                        staticData[field->slotId] = Slot(CAST_F8(0));
                        break;

                    default:
                        staticData[field->slotId] = Slot(CAST_I4(0));
                }
            }
        }
    }
    
    void InstanceClass::initSpecialType() {
        const auto threadClass = classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        if (this == threadClass || this->isSubClassOf(threadClass)) {
            specialInstanceClass = SpecialInstanceClass::THREAD_CLASS;
        }
    }

    bool InstanceClass::notInitialize() const {
        return initStatus == ClassInitStatusEnum::Loaded;
    }

    void InstanceClass::clinit(Frame &frame) {
        if (!notInitialize()) {
            return;
        }

        initStatus = ClassInitStatusEnum::Init;

        if (superClass != nullptr) {
           superClass->clinit(frame);
        }

        const auto clinitMethod = getMethod("<clinit>", "()V", true);
        if (clinitMethod != nullptr && &(clinitMethod->klass) == this) {
            frame.runMethodManual(*clinitMethod, {});
        }

        initSpecialType();

        initStatus = ClassInitStatusEnum::Inited;
    }

    BootstrapMethodsAttribute *InstanceClass::getBootstrapMethodAttr() const {
        return CAST_BOOT_STRAP_METHODS_ATTRIBUTE(bootstrapMethodsAttr.get());
    }

    EnclosingMethodAttribute *InstanceClass::getEnclosingMethodAttr() const {
        return CAST_ENCLOSING_METHOD_ATTRIBUTE(enclosingMethodAttr.get());
    }

    InnerClassesAttribute *InstanceClass::getInnerClassesAttr() const {
        return CAST_INNER_CLASSES_ATTRIBUTE(innerClassesAttr.get());
    }


    Field *InstanceClass::getField(const cstring &name, const cstring &descriptor, bool isStatic) const {
        for (auto k = this; k != nullptr; k = k->superClass) {
            for (const auto &item: k->fields) {
                if (item->is(name, descriptor, isStatic)) {
                    return item.get();
                }
            }
        }

        panic("can't find field " + name + "#" + name + ":" + descriptor);
        return nullptr;
    }


    Field *InstanceClass::getRefField(size_t refIndex, bool isStatic) const {
        return static_cast<Field *>(getMemberByRefIndex(refIndex, ClassMemberTypeEnum::FIELD, isStatic));
    }

    Method *InstanceClass::getRefMethod(size_t refIndex, bool isStatic) const {
        return static_cast<Method *>(getMemberByRefIndex(refIndex, ClassMemberTypeEnum::METHOD, isStatic));
    }

    Method *InstanceClass::getMethod(const cstring &name, const cstring &descriptor, bool isStatic) const {
        for (auto k = this; k != nullptr; k = k->superClass) {
            for (const auto &item: k->methods) {
                if (item->is(name, descriptor, isStatic)) {
                    return item.get();
                }
            }
        }

        if (isInterface()) {
            for (const auto &interface : interfaces) {
                if (const auto interfaceMethod = interface->getMethod(name, descriptor, isStatic); 
                        interfaceMethod != nullptr) {
                    return interfaceMethod;
                }
                
            }
        }

        return nullptr;
    }

    ClassMember *InstanceClass::getMemberByRefIndex(size_t refIndex, ClassMemberTypeEnum type, bool isStatic) const {
        const auto [className, memberName, memberDescriptor] = getConstantStringFromPoolByClassNameType(constantPool, refIndex);
        const auto memberClass = classLoader.getClass(className);
        const auto memberInstanceClass = 
            memberClass->isArray() ? 
                classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT) :
                CAST_INSTANCE_CLASS(memberClass);
            
        if (type == ClassMemberTypeEnum::FIELD) {
            return memberInstanceClass->getField(memberName, memberDescriptor, isStatic);
        } else {
            return memberInstanceClass->getMethod(memberName, memberDescriptor, isStatic);
        }

    }


    Slot InstanceClass::getFieldValue(const size_t index) const {
        return staticData[index];
    }

    Slot InstanceClass::getFieldValue(const cstring &name, const cstring &descriptor) const {
        auto field = getField(name, descriptor, true);
        return staticData[field->slotId];
    }

    void InstanceClass::setFieldValue(size_t index, Slot value) const {
        staticData[index] = value;
    }

    void InstanceClass::setFieldValue(const cstring &name, const cstring &descriptor, Slot value) const {
        auto field = getField(name, descriptor, true);
        staticData[field->slotId] = value;
    }

    ArrayClass::ArrayClass(const ClassTypeEnum type, const cstring &name,
                           ClassLoader &classLoader, size_t dimension) :
            InstanceClass(type, CAST_U2(AccessFlagEnum::ACC_PUBLIC), name, classLoader), dimension(dimension) {
    }

    cstring ArrayClass::getComponentClassName() const {
        auto componentClassName = name.substr(1);
        const auto firstChar = componentClassName[0];
        switch (firstChar) {
            case '[':
                return componentClassName;

            case 'L':
                return componentClassName.substr(1, componentClassName.size() - 2);

            default:
                return PRIMITIVE_TYPE_REVERSE_MAP.at(componentClassName);
        }
    }

    TypeArrayClass::TypeArrayClass(const cstring &name, ClassLoader &classLoader, size_t dimension,
                                   const BasicType elementType) :
            ArrayClass(ClassTypeEnum::TypeArrayClass, name, classLoader, dimension),
            elementType(elementType) {
    }

    ObjArrayClass::ObjArrayClass(const cstring &name, ClassLoader &classLoader, size_t dimension,
                                 const InstanceClass *elementClass) :
            ArrayClass(ClassTypeEnum::ObjArrayClass, name, classLoader, dimension),
            elementClass(elementClass) {
    }

}