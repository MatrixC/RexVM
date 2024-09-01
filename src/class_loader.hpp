#ifndef CLASS_LOADER_HPP
#define CLASS_LOADER_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include "basic_type.hpp"
#include "config.hpp"
#include "basic_java_class.hpp"

namespace RexVM {

    struct VM;
    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct TypeArrayClass;
    struct ObjArrayClass;
    struct ClassPath;
    struct ClassLoader;
    struct MirrorOop;
    struct InstanceOop;

    struct ClassLoader {
        VM &vm;
        ClassPath &classPath;
        std::unordered_map<cstring, std::unique_ptr<Class>> classMap;
        std::recursive_mutex clMutex;
        InstanceClass *mirrorClass{nullptr}; // java/lang/Class
        InstanceClass *mirrorClassLoader{nullptr};
        std::vector<InstanceClass *> basicJavaClass;
        std::atomic_int anonymousClassIndex{0};

        explicit ClassLoader(VM &vm, ClassPath &classPath);
        ~ClassLoader();
        void initBasicJavaClass();

        Class *getClass(const cstring &name);
        InstanceClass *getBasicJavaClass(BasicJavaClassEnum classEnum) const;
        InstanceClass *getInstanceClass(const cstring &name);
        ArrayClass *getArrayClass(const cstring &name);
        TypeArrayClass *getTypeArrayClass(BasicType type);
        ObjArrayClass *getObjectArrayClass(const cstring &name);
        ObjArrayClass *getObjectArrayClass(const Class &klass);
        InstanceClass *loadInstanceClass(const u1 *ptr, size_t length, bool notAnonymous);

    private:

        void initKeySlotId() const;
        void loadBasicClass();
        void loadArrayClass(const cstring &name);

        InstanceClass *loadInstanceClass(cview name);
        InstanceClass *loadInstanceClass(std::istream &is, bool notAnonymous);
        InstanceClass *loadInstanceClass(std::istream &is);

        void initMirrorClass(Class *klass);
    };


}

#endif