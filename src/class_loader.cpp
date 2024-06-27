#include "class_loader.hpp"
#include "basic_type.hpp"
#include "utils/class_path.hpp"
#include "utils/class_utils.hpp"
#include "constant_info.hpp"
#include "class_file.hpp"
#include "class.hpp"
#include "vm.hpp"
#include "oop.hpp"
#include "memory.hpp"
#include "basic_java_class.hpp"
#include <mutex>


namespace RexVM {

    ClassLoader::ClassLoader(VM &vm, ClassPath &classPath) : vm(vm), classPath(classPath) {
        loadBasicClass();
    }

    ClassLoader::~ClassLoader() = default;

    void ClassLoader::loadBasicClass() {
        std::lock_guard<std::recursive_mutex> lock(clMutex);
        for (const auto &item : PRIMITIVE_TYPE_MAP) {
            const auto name = item.first;
            auto klass = std::make_unique<Class>(
                ClassTypeEnum::PrimitiveClass,
                static_cast<u2>(AccessFlagEnum::ACC_PUBLIC),
                name,
                *this
            );
            klass->superClass = getInstanceClass(JAVA_LANG_OBJECT_NAME);
            classMap.emplace(name, std::move(klass));
        }
    
        mirrorClass = getInstanceClass(JAVA_LANG_CLASS_NAME);
        mirrorClassLoader = getInstanceClass(JAVA_LANG_CLASS_LOADER_NAME);
        for (const auto &classItem: classMap) {
            initMirrorClass(classItem.second.get());
        }
    }

    InstanceClass *ClassLoader::loadInstanceClass(std::istream &is) {
        ClassFile cf(is);
        auto instanceClass = std::make_unique<InstanceClass>(*this, cf);
        const auto rawPtr = instanceClass.get();
        initMirrorClass(rawPtr);
        classMap.emplace(instanceClass->name, std::move(instanceClass));
        return rawPtr;
    }

    InstanceClass *ClassLoader::loadInstanceClass(const cstring &name) {
        auto streamPtr = classPath.getStream(name + ".class");
        if (streamPtr == nullptr) {
            panic("Can't find class " + name);
            //throw Class Not Found expception
        }
        return loadInstanceClass(*streamPtr);
    }

    Class *ClassLoader::getClass(const cstring &name) {
        std::lock_guard<std::recursive_mutex> lock(clMutex);

        if (name == EMPTY_STRING) {
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

        return classMap[name].get();
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
            const auto basicType = getBasicType(typeStart);
            arrayClass = std::make_unique<TypeArrayClass>(name, *this, typeIndex, basicType);
            if (typeIndex != 1) {
                const auto subClassName = name.substr(1);
                const auto subClass = dynamic_cast<ArrayClass *>(getClass(subClassName));
                arrayClass->lowerDimension = subClass;
                subClass->higherDimension = arrayClass.get();
            }
        } else {
            auto elementClass = getInstanceClass(name.substr(typeIndex + 1, name.size() - typeIndex - 2));
            arrayClass = std::make_unique<ObjArrayClass>(name, *this, typeIndex, elementClass);
            if (typeIndex != 1) {
                const auto subClassName = name.substr(1);
                const auto subClass = dynamic_cast<ArrayClass *>(getClass(subClassName));
                arrayClass->lowerDimension = subClass;
                subClass->higherDimension = arrayClass.get();
            }
        }

        arrayClass->initStatus = ClassInitStatusEnum::Inited;
        arrayClass->superClass = getInstanceClass(JAVA_LANG_OBJECT_NAME);
        initMirrorClass(arrayClass.get());
        classMap.emplace(arrayClass->name, std::move(arrayClass));
    }


    void ClassLoader::initMirrorClass(Class *klass) const {
        if (klass->mirror == nullptr) {
            if (mirrorClass != nullptr) {
                /*
                auto mirror = new MirrorOop(mirrorClass, klass);
                if (classLoaderInstance != nullptr) {
                    //mirror->setFieldValue("classLoader", "Ljava/lang/ClassLoader;", Slot(classLoaderInstance));
                    //TODO bootstrapLoader or application loader is different
                }
                klass->mirror = mirror;
                */
              klass->mirror = std::make_unique<MirrorOop>(mirrorClass, klass);
            }
        }
    }


    InstanceClass *ClassLoader::getInstanceClass(const cstring &name) {
        return static_cast<InstanceClass *>(getClass(name));
    }

    ArrayClass *ClassLoader::getArrayClass(const cstring &name) {
        return static_cast<ArrayClass *>(getClass(name));
    }

    TypeArrayClass *ClassLoader::getTypeArrayClass(BasicType type) {
        const auto className = typeArrayClassName(type);
        return static_cast<TypeArrayClass *>(getClass(className));
    }

    ObjArrayClass *ClassLoader::getObjectArrayClass(const cstring &name) {
        const auto prefix = "[";
        const auto className = prefix + getDescriptorClassName(name);
        return static_cast<ObjArrayClass *>(getClass(className));
    }

    void ClassLoader::initBasicJavaClass() {
        for (const auto &item : BASIC_JAVA_CLASS_NAMES) {
            basicJavaClass.emplace_back(getInstanceClass(item));
        }
    }

    InstanceClass *ClassLoader::getBasicJavaClass(BasicJavaClassEnum classEnum) const {
        return basicJavaClass.at(static_cast<size_t>(classEnum));
    }
}