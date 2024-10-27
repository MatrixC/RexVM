#include "class.hpp"

#include <utility>
#include "basic_java_class.hpp"
#include "string_pool.hpp"
#include "class_member.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "vm.hpp"
#include "thread.hpp"
#include "frame.hpp"
#include "exception.hpp"
#include "attribute_info.hpp"
#include "utils/class_utils.hpp"

namespace RexVM {

    constexpr u2 CLASS_MEMBER_SORT_THREAHOLD = 32;

    Class::Class(const ClassTypeEnum type, const u2 accessFlags, cview name, ClassLoader &classLoader) :
            id(name, type),
            type(type), 
            accessFlags(accessFlags), 
            classLoader(classLoader) {
    }

    void Class::setName(cview name) {
        id.setName(name, type);
    }

    cview Class::getClassName() const {
        return id.getName();
    }

    cview Class::getClassDescriptor() const {
        return id.getDescritpor();
    }

    cview Class::toView() const {
        return getClassName();
    }

    ClassTypeEnum Class::getType() const {
        return type;
    }

    u2 Class::getAccessFlags() const {
        return accessFlags;
    }

    size_t Class::getInterfaceSize() const {
        return interfaces.size();
    }

    InstanceClass *Class::getInterfaceByIndex(size_t index) const {
        return interfaces.data()[index];
    }

    InstanceClass *Class::getSuperClass() const {
        return superClass;
    }

    SpecialClassEnum Class::getSpecialClassType() const {
        if (type != ClassTypeEnum::INSTANCE_CLASS) {
            return SpecialClassEnum::NONE;
        }
        return (static_cast<const InstanceClass *>(this))->specialClassType;
    }

    bool Class::isInstanceClass() const {
        return type == ClassTypeEnum::INSTANCE_CLASS;
    }

    bool Class::isInterface() const {
        return (accessFlags & CAST_U2(AccessFlagEnum::ACC_INTERFACE)) != 0;
    }

    bool Class::isArray() const {
        return type == ClassTypeEnum::TYPE_ARRAY_CLASS || type == ClassTypeEnum::OBJ_ARRAY_CLASS;
    }

    bool Class::isJavaObjectClass() const {
        return getClassName() == JAVA_LANG_OBJECT_NAME;
    }

    bool Class::isJavaCloneable() const {
        return getClassName() == JAVA_LANG_CLONEABLE_NAME;
    }

    bool Class::isSerializable() const {
        return getClassName() == JAVA_IO_SERIALIZABLE_NAME;
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

    bool Class::isInstanceOf(const Class *that) const {
        //this是父类
        //that是子类
        if (this == that) {
            return true;
        }

        if (this->isSubClassOf(that)) {
            return true;
        }

        if (this->isImplements(that)) {
            return true;
        }

        return false;
    }


    bool Class::isSubClassOf(const Class *that) const {
        for (auto c = this->getSuperClass(); c != nullptr; c = c->getSuperClass()) {
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
        for (auto c = self; c != nullptr; c = c->getSuperClass()) {
            FOR_FROM_ZERO(c->getInterfaceSize()) {
                const auto item = c->getInterfaceByIndex(i);
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
        FOR_FROM_ZERO(self->getInterfaceSize()) { 
            const auto item = self->getInterfaceByIndex(i);
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

    MirOop *Class::getMirror(Frame *frame, bool init) {
        static SpinLock lock;
        return mirrorBase.getBaseMirror(frame, MirrorObjectTypeEnum::CLASS, this, lock, init);
    }


    Class::~Class() = default;

    constexpr auto PRIMITIVE_CLASS_ACCESS_FLAGS = 
        CAST_U2(AccessFlagEnum::ACC_PUBLIC) |
        CAST_U2(AccessFlagEnum::ACC_FINAL) |
        CAST_U2(AccessFlagEnum::ACC_ABSTRACT);

    PrimitiveClass::PrimitiveClass(BasicType basicType, ClassLoader &classLoader) :
        Class(
                ClassTypeEnum::PRIMITIVE_CLASS,
                PRIMITIVE_CLASS_ACCESS_FLAGS,
                getPrimitiveClassNameByBasicType(basicType),
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
            case BasicType::T_BOOLEAN: return oop->getFieldValue("value" "Z");
            case BasicType::T_CHAR: return oop->getFieldValue("value" "C");
            case BasicType::T_FLOAT: return oop->getFieldValue("value" "F");
            case BasicType::T_DOUBLE: return oop->getFieldValue("value" "D");
            case BasicType::T_BYTE: return oop->getFieldValue("value" "B");
            case BasicType::T_SHORT: return oop->getFieldValue("value" "S");
            case BasicType::T_INT: return oop->getFieldValue("value" "I");
            case BasicType::T_LONG: return oop->getFieldValue("value" "J");
            case BasicType::T_VOID: panic("error type void");
            default:
                panic("error basicType");
                return ZERO_SLOT;
        }
    }

    InstanceOop *PrimitiveClass::getBoxingOopFromValue(Slot value, Frame &frame) const {
        switch (basicType) {
            case BasicType::T_BOOLEAN: return frame.mem.newBooleanOop(value.i4Val);
            case BasicType::T_CHAR: return frame.mem.newCharOop(value.i4Val);
            case BasicType::T_FLOAT: return frame.mem.newFloatOop(value.f4Val);
            case BasicType::T_DOUBLE: return frame.mem.newDoubleOop(value.f8Val);
            case BasicType::T_BYTE: return frame.mem.newByteOop(value.i4Val);
            case BasicType::T_SHORT: return frame.mem.newShortOop(value.i4Val);
            case BasicType::T_INT: return frame.mem.newIntegerOop(value.i4Val);
            case BasicType::T_LONG: return frame.mem.newLongOop(value.i4Val);
            case BasicType::T_VOID: panic("error type void");
            default:
                panic("error basicType");
                return nullptr;
        }
    }


    InstanceClass::InstanceClass(ClassLoader &classLoader, ClassFile &cf) :
            Class(ClassTypeEnum::INSTANCE_CLASS, cf.accessFlags, cf.getThisClassName(), classLoader) {

        sourceFile = cf.getSourceFile();
        initAttributes(cf);
        initFields(cf);
        initMethods(cf);
        initInterfaceAndSuperClass(cf);
        moveConstantPool(cf);
        calcFieldSlotId();
    }

    void InstanceClass::initAttributes(ClassFile &cf) {
        //对于ByteStreamAttribute类Attribute来说 只要把其中的字节流move出来即可
        //basicAnnotationContainer就干了这件事 所以不怕cf释放后attribute不存在
        //ByteStreamAttribute这个壳子不在了 里面的数据都被move了
        ByteStreamAttribute *annotation = nullptr;
        ByteStreamAttribute *typeAnnotation = nullptr;
        //其他类型的atribute 需要直接move出来


        for (auto &attribute: cf.attributes) {
            const auto attributeName = getConstantStringFromPool(cf.constantPool, CAST_SIZE_T(attribute->attributeNameIndex));
            const auto attributeTypeEnum = ATTRIBUTE_NAME_TAG_MAP.at(attributeName);
            switch (attributeTypeEnum) {
                case AttributeTagEnum::BOOTSTRAP_METHODS: {
                    if (classAttributeContainer == nullptr) {
                        classAttributeContainer = std::make_unique<ClassAttributeContainer>();
                    }
                    classAttributeContainer->bootstrapMethodsAttr = std::move(attribute);
                    break;
                }

                case AttributeTagEnum::ENCLOSING_METHOD: {
                    if (classAttributeContainer == nullptr) {
                        classAttributeContainer = std::make_unique<ClassAttributeContainer>();
                    }
                    classAttributeContainer->enclosingMethodAttr = std::move(attribute);
                    break;
                }

                case AttributeTagEnum::INNER_CLASSES: {
                    if (classAttributeContainer == nullptr) {
                        classAttributeContainer = std::make_unique<ClassAttributeContainer>();
                    }
                    classAttributeContainer->innerClassesAttr = std::move(attribute);
                    break;
                }

                case AttributeTagEnum::RUNTIME_VISIBLE_ANNOTATIONS: {
                    annotation = CAST_BYTE_STREAM_ATTRIBUTE(attribute.get());
                    break;
                }

                case AttributeTagEnum::RUNTIME_VISIBLE_TYPE_ANNOTATIONS: {
                    typeAnnotation = CAST_BYTE_STREAM_ATTRIBUTE(attribute.get());
                    break;
                }

                case AttributeTagEnum::SIGNATURE: {
                    signatureIndex = (CAST_SIGNATURE_ATTRIBUTE(attribute.get()))->signatureIndex;
                    break;
                }

                default:
                    break;
            }
        }

        if (annotation != nullptr || typeAnnotation != nullptr) {
            basicAnnotationContainer = std::make_unique<BasicAnnotationContainer>(annotation, typeAnnotation);
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
        if (fields.size() > CLASS_MEMBER_SORT_THREAHOLD) {
            std::sort(fields.begin(), fields.end(), Field::compare);
        }
    }

    void InstanceClass::initMethods(ClassFile &cf) {
        methods.reserve(cf.methodCount);
        for (const auto &methodInfo: cf.methods) {
            auto method = std::make_unique<Method>(*this, methodInfo.get(), cf);
            if (!overrideFinalize && !isJavaObjectClass() && method->isFinalize()) {
                overrideFinalize = true;
            }
            methods.emplace_back(std::move(method));
        }
        if (methods.size() > CLASS_MEMBER_SORT_THREAHOLD) {
            std::sort(methods.begin(), methods.end(), Method::compare);
        }
        u2 index = 0;
        for (const auto &item : methods) {
            item->slotId = index++;
        }
    }

    void InstanceClass::initInterfaceAndSuperClass(ClassFile &cf) {
        // using InstanceClassPtr = InstanceClass *;
        // auto interfaceArray = new InstanceClassPtr[cf.interfaceCount];
        // const auto &interfaceNames = cf.getInterfaceNames();
        // FOR_FROM_ZERO(interfaceNames.size()) {
        //     interfaceArray[i] = classLoader.getInstanceClass(interfaceNames[i]);
        // }

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
        std::ranges::move(cf.constantPool, std::back_inserter(constantPool));
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

        instanceDataType = std::make_unique<SlotTypeEnum[]>(instanceSlotCount);
        staticDataType = std::make_unique<SlotTypeEnum[]>(staticSlotCount);
        staticData = std::make_unique<Slot[]>(staticSlotCount);

        for (auto klass = this; klass != nullptr; klass = klass->getSuperClass()) {
            for (const auto &field: klass->fields) {
                if (field->isStatic()) {
                    if (klass == this) {
                        staticDataType[field->slotId] = field->getFieldSlotType();
                    }
                } else {
                    instanceDataType[field->slotId] = field->getFieldSlotType();
                }
            } 
        }
    }

    void InstanceClass::initStaticField(VMThread &thread) {
        if (staticSlotCount <= 0) {
            return;
        }
        // staticData = std::make_unique<Slot[]>(staticSlotCount);
        for (const auto &field: fields) {
            if (!field->isStatic()) {
                continue;
            }
            const auto slotType = field->getFieldSlotType();
            if (field->isFinal() && field->constantValueIndex > 0) {
                const auto &constValue = constantPool[field->constantValueIndex];
                const auto descriptor = field->getDescriptor();
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
                        &thread,
                        getConstantStringFromPoolByIndexInfo(constantPool, field->constantValueIndex)
                    );
                    data = Slot(strOop);
                } else {
                    panic("error descriptor");
                }
                staticData[field->slotId] = data;
            } else {
                switch (slotType) {
                    case SlotTypeEnum::I4:
                        staticData[field->slotId] = Slot(CAST_I4(0));
                        break;
                    case SlotTypeEnum::F4:
                        staticData[field->slotId] = Slot(CAST_F4(0));
                        break;
                    case SlotTypeEnum::I8:
                        staticData[field->slotId] = Slot(CAST_I8(0));
                        break;
                    case SlotTypeEnum::F8:
                        staticData[field->slotId] = Slot(CAST_F8(0));
                        break;
                    case SlotTypeEnum::REF:
                        staticData[field->slotId] = Slot(nullptr);
                        break;
                    default:
                        panic("error slotType");
                        break;
                }
            }
        }
    }
    
    void InstanceClass::initSpecialType() {
        const auto threadClass = classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        const auto memberNameClass = classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INVOKE_MEMBER_NAME);
        if (this == threadClass || this->isSubClassOf(threadClass)) {
            specialClassType = SpecialClassEnum::THREAD_CLASS;
        } else if (this == memberNameClass) {
            specialClassType = SpecialClassEnum::MEMBER_NAME_CLASS;
        }
    }

    bool InstanceClass::notInitialize() const {
        return initStatus == ClassInitStatusEnum::LOADED;
    }

    void InstanceClass::clinit(Frame &frame) {
        if (!notInitialize()) {
            return;
        }

        std::lock_guard guard(initLock);
        if (!notInitialize()) {
            return;
        }

        initStatus = ClassInitStatusEnum::INIT;

        if (superClass != nullptr) {
           superClass->clinit(frame);
            if (frame.markThrow) {
                //最深层以外在这里异常
                initStatus = ClassInitStatusEnum::LOADED;
                return;
            }
        }

        initStaticField(frame.thread);

        const auto clinitMethod = getMethod("<clinit>" "()V", true);
        if (clinitMethod != nullptr && &(clinitMethod->klass) == this) {
            frame.runMethodManual(*clinitMethod, {});
            if (frame.markThrow) {
                //最深层的clinit在此处异常
                initStatus = ClassInitStatusEnum::LOADED;
                return;
            }
        }

        initSpecialType();

        initStatus = ClassInitStatusEnum::INITED;
    }

    BootstrapMethodsAttribute *InstanceClass::getBootstrapMethodAttr() const {
        return classAttributeContainer == nullptr ? nullptr :
                CAST_BOOT_STRAP_METHODS_ATTRIBUTE(classAttributeContainer->bootstrapMethodsAttr.get());
    }

    EnclosingMethodAttribute *InstanceClass::getEnclosingMethodAttr() const {
        return classAttributeContainer == nullptr ? nullptr :
            CAST_ENCLOSING_METHOD_ATTRIBUTE(classAttributeContainer->enclosingMethodAttr.get());
    }

    InnerClassesAttribute *InstanceClass::getInnerClassesAttr() const {
        return classAttributeContainer == nullptr ? nullptr :
            CAST_INNER_CLASSES_ATTRIBUTE(classAttributeContainer->innerClassesAttr.get());
    }

    template<typename T>
    T *getClassMemberSelf(cview id, bool isStatic, const std::vector<std::unique_ptr<T>> &members) {
        const auto sortMember = members.size() > CLASS_MEMBER_SORT_THREAHOLD;
        if (sortMember) [[unlikely]] {
            //排序过的使用二分查找
            const auto iter = 
                std::lower_bound(
                    members.begin(), 
                    members.end(), 
                    id, 
                    [](const std::unique_ptr<T> &a, const cview &n) { return a->id.id < n; }
                );
            
            if (iter != members.end() && (*iter)->id.id == id) {
                return (*iter).get();
            }
        } else {
            for (const auto &item : members) {
                if (item->is(id, isStatic)) {
                    return item.get();
                }
            }
        }

        return nullptr;
    }
    
    Field *InstanceClass::getFieldSelf(cview id, bool isStatic) const {
        return getClassMemberSelf(id, isStatic, fields);
    }

    Field *InstanceClass::getField(cview id, bool isStatic) const {
        if (const auto selfMember = getFieldSelf(id, isStatic); selfMember != nullptr) {
            return selfMember;
        }

        if (superClass != nullptr) {
            if (const auto field = superClass->getField(id, isStatic); field != nullptr) {
                return field;
            }
        }

        for (const auto &interface : interfaces) {
            if (const auto interfaceField = interface->getField(id, isStatic); interfaceField != nullptr) {
                return interfaceField;
            }
        }

        return nullptr;
    }

    Field *InstanceClass::getField(cview name, cview descriptor, bool isStatic) const {
        rstring combineId(name, descriptor);
        return getField(combineId.toStringView(), isStatic);
    }

    Field *InstanceClass::getRefField(size_t refIndex, bool isStatic) const {
        return static_cast<Field *>(getMemberByRefIndex(refIndex, ClassMemberTypeEnum::FIELD, isStatic));
    }

    Method *InstanceClass::getRefMethod(size_t refIndex, bool isStatic) const {
        return static_cast<Method *>(getMemberByRefIndex(refIndex, ClassMemberTypeEnum::METHOD, isStatic));
    }

    cview InstanceClass::getSignature() const {
        if (signatureIndex == 0) {
            return {};
        }
        return getConstantStringFromPool(constantPool, signatureIndex);
    }

    Method *InstanceClass::getMethodSelf(cview id, bool isStatic) const {
        return getClassMemberSelf(id, isStatic, methods);
    }

    Method *InstanceClass::getMethod(cview id, bool isStatic) const {
        if (const auto selfMember = getMethodSelf(id, isStatic); selfMember != nullptr) {
            return selfMember;
        }

        if (superClass != nullptr) {
            if (const auto method = superClass->getMethod(id, isStatic); method != nullptr) {
                return method;
            }
        }

        for (const auto &interface : interfaces) {
            if (const auto interfaceMethod = interface->getMethod(id, isStatic); 
                    interfaceMethod != nullptr) {
                return interfaceMethod;
            }
        }

        return nullptr;
    }

    Method *InstanceClass::getMethod(cview name, cview descriptor, bool isStatic) const {
        rstring combineId(name, descriptor);
        return getMethod(combineId.toStringView(), isStatic);
    }

    ClassMember *InstanceClass::getMemberByRefIndex(size_t refIndex, ClassMemberTypeEnum type, bool isStatic) const {
        const auto [className, memberName, memberDescriptor] = getConstantStringFromPoolByClassNameType(constantPool, refIndex);
        const auto memberClass = classLoader.getClass(className);
        const auto memberInstanceClass = 
            memberClass->isArray() ? 
                classLoader.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT) :
                CAST_INSTANCE_CLASS(memberClass);

        rstring id(memberName, memberDescriptor);
        
        if (type == ClassMemberTypeEnum::FIELD) {
            return memberInstanceClass->getField(id.toStringView(), isStatic);
        } else {
            return memberInstanceClass->getMethod(id.toStringView(), isStatic);
        }
    }

    Slot InstanceClass::getFieldValue(const size_t index) const {
        return staticData[index];
    }

    Slot InstanceClass::getFieldValue(cview id) const {
        auto field = getField(id, true);
        return staticData[field->slotId];
    }

    void InstanceClass::setFieldValue(size_t index, Slot value) const {
        staticData[index] = value;
    }

    void InstanceClass::setFieldValue(cview id, Slot value) const {
        auto field = getField(id, true);
        staticData[field->slotId] = value;
    }

    MirOop *InstanceClass::getConstantPoolMirror(Frame *frame, bool init) {
        static SpinLock lock;
        return constantPoolMirrorBase.getBaseMirror(frame, MirrorObjectTypeEnum::CONSTANT_POOL, this, lock, init);
    }

    ArrayClass::ArrayClass(const ClassTypeEnum type, cview name,
                           ClassLoader &classLoader, size_t dimension) :
            //InstanceClass(type, CAST_U2(AccessFlagEnum::ACC_PUBLIC), name, classLoader), 
            Class(type, CAST_U2(AccessFlagEnum::ACC_PUBLIC), name, classLoader),
            dimension(dimension) {
    }

    cview ArrayClass::getComponentClassName() const {
        auto componentClassName = getClassName().substr(1);
        return getClassNameByFieldDescriptor(componentClassName);
    }

    TypeArrayClass::TypeArrayClass(cview name, ClassLoader &classLoader, size_t dimension,
                                   const BasicType elementType) :
            ArrayClass(ClassTypeEnum::TYPE_ARRAY_CLASS, name, classLoader, dimension),
            elementType(elementType) {
    }

    ObjArrayClass::ObjArrayClass(cview name, ClassLoader &classLoader, size_t dimension,
                                 const InstanceClass *elementClass) :
            ArrayClass(ClassTypeEnum::OBJ_ARRAY_CLASS, name, classLoader, dimension),
            elementClass(elementClass) {
    }

}