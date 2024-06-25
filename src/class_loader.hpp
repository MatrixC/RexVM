#ifndef CLASS_LOADER_HPP
#define CLASS_LOADER_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "basic_type.hpp"
#include "config.hpp"

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
        static InstanceClass *mirrorClass; // java/lang/Class
        static InstanceClass *mirrorClassLoader;

        Class *getClass(const cstring &name);

        InstanceClass *getInstanceClass(const cstring &name);

        ArrayClass *getArrayClass(const cstring &name);

        TypeArrayClass *getTypeArrayClass(BasicType type);

        ObjArrayClass *getObjectArrayClass(const cstring &name);

        void initMirrorClass(Class *klass) const;

        explicit ClassLoader(VM &vm, ClassPath &classPath);

        ~ClassLoader();

    private:
        void loadBasicClass();

        void loadArrayClass(const cstring &name);

        void loadInstanceClass(const cstring &name);

        void loadInstanceClass(std::istream &is);
    };


}

#endif