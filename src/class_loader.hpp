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

    struct CViewHash {
        size_t operator()(cview cv) const noexcept {
            return std::hash<cview>{}(cv);
        }
    };

    struct CViewEqual {
        bool operator()(cview lhs, cview rhs) const noexcept {
            return lhs == rhs;
        }
    };

    struct ClassLoader {
        VM &vm;
        ClassPath &classPath;
        std::unordered_map<cstring, std::unique_ptr<Class>> classMap;
        std::unordered_map<cview, std::unique_ptr<Class>, CViewHash, CViewEqual> classMap2;
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

        template<typename T>
        InstanceClass *getInstanceClass(T name) {
            return CAST_INSTANCE_CLASS(getClass(name));
        }

        template<typename T>
        ArrayClass *getArrayClass(T name) {
            return CAST_ARRAY_CLASS(getClass(name));
        }

        TypeArrayClass *getTypeArrayClass(BasicType type);
        ObjArrayClass *getObjectArrayClass(const Class &klass);
        InstanceClass *loadInstanceClass(const u1 *ptr, size_t length, bool notAnonymous);


        Class *getClass(cview name);

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