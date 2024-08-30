#include "class_loader.hpp"
#include <mutex>
#include <memory>
#include <sstream>
#include "basic_type.hpp"
#include "utils/class_path.hpp"
#include "utils/class_utils.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "vm.hpp"
#include "oop.hpp"
#include "memory.hpp"
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
        for (const auto item : PRIMITIVE_TYPE_ARRAY) {
            auto klass = std::make_unique<PrimitiveClass>(item, *this);
            klass->superClass = getInstanceClass(JAVA_LANG_OBJECT_NAME);
            classMap.emplace(klass->name, std::move(klass));
        }
    
        mirrorClass = getInstanceClass(JAVA_LANG_CLASS_NAME);
        mirrorClassLoader = getInstanceClass(JAVA_LANG_CLASS_LOADER_NAME);
        for (const auto &classItem: classMap) {
            initMirrorClass(classItem.second.get());
        }

        //load RexVM runtime class
        loadInstanceClass(REX_PRINT_STREAM_CLASS_FILE.data(), REX_PRINT_STREAM_CLASS_FILE.size(), false);
    }

    constexpr auto ANONYMOUS_CLASS_NAME_PREFIX = "ANONYMOUS";
    InstanceClass *ClassLoader::loadInstanceClass(std::istream &is, bool notAnonymous) {
        ClassFile cf(is);
        auto instanceClass = std::make_unique<InstanceClass>(*this, cf);
        const auto rawPtr = instanceClass.get();
        initMirrorClass(rawPtr);
        //const auto className = notAnonymous ? instanceClass->name : ANONYMOUS_CLASS_NAME_PREFIX + std::to_string(anonymousClassIndex.fetch_add(1));
        auto className = instanceClass->name;
        if (!notAnonymous && classMap.contains(className)) {
            className = ANONYMOUS_CLASS_NAME_PREFIX + std::to_string(anonymousClassIndex.fetch_add(1));
        }
        classMap.emplace(className, std::move(instanceClass));
        return rawPtr;
    }

    InstanceClass *ClassLoader::loadInstanceClass(std::istream &is) {
        return loadInstanceClass(is, true);
    }

    InstanceClass *ClassLoader::loadInstanceClass(cview name) {
        cstring fileName;
        const auto suffix = ".class";
        fileName.reserve(name.size() + 7);
        fileName.append(name);
        fileName.append(suffix);
        auto streamPtr = classPath.getStream(fileName);
        if (streamPtr == nullptr) {
            return nullptr;
        }
        return loadInstanceClass(*streamPtr);
    }

    Class *ClassLoader::getClass(const cstring &name) {
        std::lock_guard<std::recursive_mutex> lock(clMutex);

        if (name.empty()) {
            return nullptr;
        }
        if (const auto iter = classMap.find(name); iter != classMap.end()) {
            return iter->second.get();
        }

        if (name[0] == '[') {
            loadArrayClass(name);
        } else {
            loadInstanceClass(name);
        }

        if (const auto iter = classMap.find(name); iter != classMap.end()) {
            return iter->second.get();
        }

        return nullptr;
    }


    void ClassLoader::loadArrayClass(const cstring &name) {
        size_t typeIndex = 0;
        auto typeStart = name[0];
        for (; typeIndex < name.size(); ++typeIndex) {
            typeStart = name[typeIndex];
            if (typeStart != '[') {
                break;
            }
        }

        std::unique_ptr<ArrayClass> arrayClass;
        if (isBasicType(typeStart) && name.size() == 2) {
            const auto basicType = getBasicTypeByDescriptor(typeStart);
            arrayClass = std::make_unique<TypeArrayClass>(name, *this, typeIndex, basicType);
            if (typeIndex != 1) {
                const auto subClassName = name.substr(1);
                const auto subClass = CAST_ARRAY_CLASS(getClass(subClassName));
                arrayClass->lowerDimension = subClass;
                subClass->higherDimension = arrayClass.get();
            }
        } else {
            auto elementClass = getInstanceClass(name.substr(typeIndex + 1, name.size() - typeIndex - 2));
            arrayClass = std::make_unique<ObjArrayClass>(name, *this, typeIndex, elementClass);
            if (typeIndex != 1) {
                const auto subClassName = name.substr(1);
                const auto subClass = CAST_ARRAY_CLASS(getClass(subClassName));
                arrayClass->lowerDimension = subClass;
                subClass->higherDimension = arrayClass.get();
            }
        }

        arrayClass->initStatus = ClassInitStatusEnum::INITED;
        arrayClass->superClass = getInstanceClass(JAVA_LANG_OBJECT_NAME);
        initMirrorClass(arrayClass.get());
        classMap.emplace(arrayClass->name, std::move(arrayClass));
    }


    void ClassLoader::initMirrorClass(Class *klass) {
        //lazy init
        // if (klass->mirror == nullptr) {
        //     if (mirrorClass != nullptr) {
        //         /*
        //         auto mirror = new MirrorOop(mirrorClass, klass);
        //         if (classLoaderInstance != nullptr) {
        //             //mirror->setFieldValue("classLoader", "Ljava/lang/ClassLoader;", Slot(classLoaderInstance));
        //             //TODO bootstrapLoader or application loader is different
        //         }
        //         klass->mirror = mirror;
        //         */
        //         klass->mirror = std::make_unique<MirrorOop>(mirrorClass, klass);
        //     }
        // }
    }

    InstanceClass *ClassLoader::getInstanceClass(const cstring &name) {
        return CAST_INSTANCE_CLASS(getClass(name));
    }

    ArrayClass *ClassLoader::getArrayClass(const cstring &name) {
        return CAST_ARRAY_CLASS(getClass(name));
    }

    TypeArrayClass *ClassLoader::getTypeArrayClass(BasicType type) {
        const auto className = getTypeArrayClassNameByBasicType(type);
        return CAST_TYPE_ARRAY_CLASS(getClass(className));
    }

    ObjArrayClass *ClassLoader::getObjectArrayClass(const cstring &name) {
        const auto firstChar = name[0];
        const auto prefix = "[";
        const auto elementName = firstChar == '[' ? name : getDescriptorClassName(name);
        const auto arrayClassName = prefix + elementName;
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
                ->getField("value", "[C", false)->slotId;

        const auto threadClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD);
        threadClassThreadStatusFieldSlotId = threadClass->getField("threadStatus", "I", false)->slotId;
        threadClassExitMethodSlotId = threadClass->getMethod("exit", "()V", false)->slotId;
        threadClassDeamonFieldSlotId = threadClass->getField("daemon", "Z", false)->slotId;
        threadClassNameFieldSlotId = threadClass->getField("name", "Ljava/lang/String;", false)->slotId;

        const auto stackTraceElementClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STACK_TRACE_ELEMENT);
        steClassDeclaringClassFId = stackTraceElementClass->getField("declaringClass", "Ljava/lang/String;", false)->slotId;
        steClassMethodNameFId = stackTraceElementClass->getField("methodName", "Ljava/lang/String;", false)->slotId;
        steClassFileNameFId = stackTraceElementClass->getField("fileName", "Ljava/lang/String;", false)->slotId;
        steClassLineNumberFId = stackTraceElementClass->getField("lineNumber", "I", false)->slotId;

        const auto throwableClass = getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THROWABLE);
        throwableClassDetailMessageFieldSlotId = throwableClass->getField("detailMessage", "Ljava/lang/String;", false)->slotId;
        throwableClassBacktraceFID = throwableClass->getField("backtrace", "Ljava/lang/Object;", false)->slotId;
        throwableClassStacktraceFID = throwableClass->getField("stackTrace", "[Ljava/lang/StackTraceElement;", false)->slotId;
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
