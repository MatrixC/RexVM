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

namespace RexVM {

    Class::Class(const ClassTypeEnum type, const u2 accessFlags, cstring name, ClassLoader &classLoader) :
            type(type), accessFlags(accessFlags), name(std::move(name)), classLoader(classLoader) {
    }

    bool Class::isInstanceClass() const {
        return type == ClassTypeEnum::InstanceClass;
    }

    bool Class::isInterface() const {
        return (accessFlags & static_cast<u2>(AccessFlagEnum::ACC_INTERFACE)) != 0;
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
        const auto s = that;
        const auto t = this;

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

    MirrorOop * Class::getMirrorOop() const {
        return mirror.get();
    }


    Class::~Class() = default;

    InstanceClass::InstanceClass(ClassLoader &classLoader, ClassFile &cf) :
            Class(ClassTypeEnum::InstanceClass, cf.accessFlags, cf.getThisClassName(), classLoader) {

        sourceFile = cf.getSourceFile();

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

        methods.reserve(cf.methodCount);
        for (const auto &methodInfo: cf.methods) {
            auto method =
                    std::make_unique<Method>(
                            *this,
                            methodInfo.get(),
                            cf
                    );
            methods.emplace_back(std::move(method));
        }

        interfaces.reserve(cf.interfaceCount);
        for (const auto &interfaceName: cf.getInterfaceNames()) {
            interfaces.emplace_back(classLoader.getInstanceClass(interfaceName));
        }

        superClass = classLoader.getInstanceClass(cf.getSuperClassName());
        constantPool.reserve(cf.constantPool.size());
        std::move(cf.constantPool.begin(), cf.constantPool.end(), std::back_inserter(constantPool));

        calcFieldSlotId();
        initStaticField();
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
            frame.runMethod(*clinitMethod);
        }

        initStatus = ClassInitStatusEnum::Inited;
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
                    data = Slot(static_cast<ConstantIntegerInfo *>(constValue.get())->value);
                } else if (descriptor == "J") {
                    data = Slot(static_cast<ConstantLongInfo *>(constValue.get())->value);
                } else if (descriptor == "F") {
                    data = Slot(static_cast<ConstantFloatInfo *>(constValue.get())->value);
                } else if (descriptor == "D") {
                    data = Slot(static_cast<ConstantDoubleInfo *>(constValue.get())->value);
                } else if (descriptor == "Ljava/lang/String;") {
                    auto stringConstInfo = static_cast<ConstantStringInfo *>(constValue.get());
                    const auto &utf8Value = constantPool.at(stringConstInfo->index);
                    auto strOop = classLoader.vm.stringPool->getInternString(
                            static_cast<ConstantUTF8Info *>(utf8Value.get())->str
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
                        staticData[field->slotId] = Slot(static_cast<f4>(0));
                        break;

                    case SlotTypeEnum::F8:
                        staticData[field->slotId] = Slot(static_cast<f8>(0));
                        break;

                    default:
                        staticData[field->slotId] = Slot(static_cast<i4>(0));
                }
            }
        }
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
            
        return nullptr;
    }

    ClassMember *InstanceClass::getMemberByRefIndex(size_t refIndex, ClassMemberTypeEnum type, bool isStatic) const {
        const auto memberInfo = static_cast<ConstantClassNameTypeIndexInfo *>(constantPool.at(refIndex).get());
        const auto classInfo = static_cast<ConstantClassInfo *>(constantPool.at(memberInfo->classIndex).get());
        const auto className = getConstantStringFromPool(constantPool, classInfo->index);
        const auto nameAndTypeInfo = static_cast<ConstantNameAndTypeInfo *>(constantPool.at(
                memberInfo->nameAndTypeIndex).get());
        const auto memberName = getConstantStringFromPool(constantPool, nameAndTypeInfo->nameIndex);
        const auto memberDescriptor = getConstantStringFromPool(constantPool, nameAndTypeInfo->descriptorIndex);
        const auto memberClass = classLoader.getClass(className);
        const auto memberInstanceClass = 
            memberClass->isArray() ? 
                //classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT) :
                classLoader.getInstanceClass("java/lang/Object") : 
                classLoader.getInstanceClass(className);
            
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
            InstanceClass(type, static_cast<u2>(AccessFlagEnum::ACC_PUBLIC), name, classLoader), dimension(dimension) {
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