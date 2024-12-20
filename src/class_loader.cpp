#include "class_loader.hpp"
#include <mutex>
#include <memory>
#include <sstream>
#include "basic_type.hpp"
#include "utils/class_path.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "basic_java_class.hpp"
#include "key_slot_id.hpp"
#include "native/rex/rex_classes.hpp"

namespace RexVM {

    ClassLoader::ClassLoader(VM &vm, ClassPath &classPath) : vm(vm), classPath(classPath) {
        loadBasicClass();
    }

    ClassLoader::~ClassLoader() = default;

    void ClassLoader::loadBasicClass() {
        std::lock_guard<std::recursive_mutex> lock(clMutex);
        const auto objectClass = getClass(JAVA_LANG_OBJECT_NAME);
        for (const auto item : PRIMITIVE_TYPE_ARRAY) {
            auto klass = std::make_unique<PrimitiveClass>(item, *this);
            klass->superClass = CAST_INSTANCE_CLASS(objectClass);
            classMap.emplace_unique(klass->getClassName(), std::move(klass));
        }

        //load RexVM runtime class
        loadInstanceClass(REX_PRINT_STREAM_CLASS_FILE.data(), REX_PRINT_STREAM_CLASS_FILE.size(), false);
    }

    constexpr auto ANONYMOUS_CLASS_NAME_PREFIX = "ANONYMOUS";
    InstanceClass *ClassLoader::loadInstanceClass(std::istream &is, bool notAnonymous) {
        ClassFile cf(is);
        auto instanceClass = std::make_unique<InstanceClass>(*this, cf);
        const auto rawPtr = instanceClass.get();
        auto className = instanceClass->getClassName();
        if (!notAnonymous && classMap.contains(className)) {
            const auto newClassName = ANONYMOUS_CLASS_NAME_PREFIX + std::to_string(anonymousClassIndex.fetch_add(1));
            instanceClass->setName(newClassName);
        }
        classMap.emplace_unique(instanceClass->getClassName(), std::move(instanceClass));
        return rawPtr;
    }

    InstanceClass *ClassLoader::loadInstanceClass(cview name) {
        const auto fileName = cformat("{}.class", name);
        auto streamPtr = classPath.getStream(fileName);
        if (streamPtr == nullptr) {
            return nullptr;
        }
        return loadInstanceClass(*streamPtr, true);
    }

    Class *ClassLoader::getClass(cview name) {
        std::lock_guard<std::recursive_mutex> lock(clMutex);

        if (name.empty()) {
            return nullptr;
        }

        const auto iter = classMap.try_get(name);
        if (iter != nullptr) {
            return (*iter).get();
        }

        if (name[0] == '[') {
            return loadArrayClass(name);
        } else {
            return loadInstanceClass(name);
        }

        panic("error name");

        return nullptr;
    }

    ArrayClass *ClassLoader::loadArrayClass(cview name) {
        const auto nameSize = name.size();
        size_t typeIndex = 0;
        auto typeStart = name[0];
        for (; typeIndex < nameSize; ++typeIndex) {
            typeStart = name[typeIndex];
            if (typeStart != '[') {
                break;
            }
        }

        std::unique_ptr<ArrayClass> arrayClass;
        if (isBasicType(typeStart) && nameSize == 2) {
            const auto basicType = getBasicTypeByDescriptor(typeStart);
            arrayClass = std::make_unique<TypeArrayClass>(name, *this, typeIndex, basicType);
        } else {
            //elementClass can lazy
            //auto elementClass = getInstanceClass(name.substr(typeIndex + 1, nameSize - typeIndex - 2));
            InstanceClass *elementClass = nullptr; 
            arrayClass = std::make_unique<ObjArrayClass>(name, *this, typeIndex, elementClass);
        }

        // if (typeIndex != 1) {
        //     const auto subClassName = name.substr(1);
        //     const auto subClass = CAST_ARRAY_CLASS(getClass(subClassName));
        //     arrayClass->lowerDimension = subClass;
        //     subClass->higherDimension = arrayClass.get();
        // }

        arrayClass->initStatus = ClassInitStatusEnum::INITED;
        const auto objectClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT);
        arrayClass->superClass = objectClass;
        const auto rawPtr = arrayClass.get();
        classMap.emplace_unique(arrayClass->getClassName(), std::move(arrayClass));
        return rawPtr;
    }

    TypeArrayClass *ClassLoader::getTypeArrayClass(BasicType type) {
        const auto className = getTypeArrayClassNameByBasicType(type);
        return CAST_TYPE_ARRAY_CLASS(getClass(className));
    }

    ObjArrayClass *ClassLoader::getObjectArrayClass(const Class &klass) {
        const auto arrayClassName = cformat("[{}", klass.getClassDescriptor());
        return CAST_OBJ_ARRAY_CLASS(getClass(arrayClassName));
    }

    void ClassLoader::initBasicJavaClass() {
        for (const auto &item : BASIC_JAVA_CLASS_NAMES) {
            basicJavaClass.emplace_back(getInstanceClass(item));
        }

        initKeySlotId();
    }

    void ClassLoader::initKeySlotId() const {
        stringClassValueFieldSlotId =
            getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STRING)
                ->getFieldSelf("value" "[C", false)->slotId;

        const auto threadClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        threadClassThreadStatusFieldSlotId = threadClass->getField("threadStatus" "I", false)->slotId;
        threadClassExitMethodSlotId = threadClass->getMethodSelf("exit" "()V", false)->slotId;
        threadClassDeamonFieldSlotId = threadClass->getFieldSelf("daemon" "Z", false)->slotId;
        threadClassNameFieldSlotId = threadClass->getFieldSelf("name" "Ljava/lang/String;", false)->slotId;

        const auto stackTraceElementClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STACK_TRACE_ELEMENT);
        steClassDeclaringClassFId = stackTraceElementClass->getFieldSelf("declaringClass" "Ljava/lang/String;", false)->slotId;
        steClassMethodNameFId = stackTraceElementClass->getFieldSelf("methodName" "Ljava/lang/String;", false)->slotId;
        steClassFileNameFId = stackTraceElementClass->getFieldSelf("fileName" "Ljava/lang/String;", false)->slotId;
        steClassLineNumberFId = stackTraceElementClass->getFieldSelf("lineNumber" "I", false)->slotId;

        const auto throwableClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THROWABLE);
        throwableClassDetailMessageFieldSlotId = throwableClass->getFieldSelf("detailMessage" "Ljava/lang/String;", false)->slotId;
        throwableClassBacktraceFID = throwableClass->getFieldSelf("backtrace" "Ljava/lang/Object;", false)->slotId;
        throwableClassStacktraceFID = throwableClass->getFieldSelf("stackTrace" "[Ljava/lang/StackTraceElement;", false)->slotId;

        const auto objectClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT);
        objClassCloneMID = objectClass->getMethodSelf("clone" "()Ljava/lang/Object;", false)->slotId;
        objClassGetClassMID = objectClass->getMethodSelf("getClass" "()Ljava/lang/Class;", false)->slotId;
    }

    InstanceClass *ClassLoader::getBasicJavaClass(BasicJavaClassEnum classEnum) const {
        return basicJavaClass[CAST_SIZE_T(classEnum)];
    }

    InstanceClass *ClassLoader::loadInstanceClass(const u1 *ptr, size_t length, bool notAnonymous) {
        cstring buffer(length, 0);
        auto bufferPtr = CAST_VOID_PTR(buffer.data());
        std::memcpy(bufferPtr, ptr, length * sizeof(u1));
        const auto classStream = std::make_unique<std::istringstream>(buffer);
        return loadInstanceClass(*classStream, notAnonymous);
    }

}
