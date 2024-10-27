#ifndef MEMORY_HPP
#define MEMORY_HPP
#include "basic.hpp"
#include <unordered_set>
#include <map>
#include <vector>
#include <atomic>
#include "utils/spin_lock.hpp"

namespace RexVM {

    struct VM;
    struct VMThread;
    struct InstanceOop;
    struct MirOop;
    struct ObjArrayOop;
    struct TypeArrayOop;
    struct ByteTypeArrayOop;
    struct CharTypeArrayOop;
    struct InstanceClass;
    struct ObjArrayClass;

    struct OopHolder {
        std::vector<ref> oops;

        explicit OopHolder(size_t size);
        explicit OopHolder();

        void addOop(ref oop);
        void clear();

        void addAnotherHolderOops(OopHolder &that);
    };

    struct OopManager {

        VM &vm;
        OopHolder defaultOopHolder{};

        explicit OopManager(VM &vm);

        [[nodiscard]] InstanceOop *newInstance(VMThread *thread, InstanceClass * klass);
        [[nodiscard]] MirOop *newMirror(VMThread *thread, InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type);
        [[nodiscard]] ObjArrayOop *newObjArrayOop(VMThread *thread, ObjArrayClass * klass, size_t length);

        [[nodiscard]] ObjArrayOop *newObjectObjArrayOop(VMThread *thread, size_t length);
        [[nodiscard]] ObjArrayOop *newClassObjArrayOop(VMThread *thread, size_t length);
        [[nodiscard]] ObjArrayOop *newStringObjArrayOop(VMThread *thread, size_t length);

        [[nodiscard]] TypeArrayOop *newTypeArrayOop(VMThread *thread, BasicType type, size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(VMThread *thread, size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(VMThread *thread, size_t length, const u1 *initBuffer);
        [[nodiscard]] CharTypeArrayOop *newCharArrayOop(VMThread *thread, size_t length);

        [[nodiscard]] InstanceOop *newBooleanOop(VMThread *thread, i4 value);
        [[nodiscard]] InstanceOop *newByteOop(VMThread *thread, i4 value);
        [[nodiscard]] InstanceOop *newCharOop(VMThread *thread, i4 value);
        [[nodiscard]] InstanceOop *newShortOop(VMThread *thread, i4 value);
        [[nodiscard]] InstanceOop *newIntegerOop(VMThread *thread, i4 value);
        [[nodiscard]] InstanceOop *newFloatOop(VMThread *thread, f4 value);
        [[nodiscard]] InstanceOop *newLongOop(VMThread *thread, i8 value);
        [[nodiscard]] InstanceOop *newDoubleOop(VMThread *thread, f8 value);
        [[nodiscard]] InstanceOop *newStringOop(VMThread *thread, CharTypeArrayOop *value);

        void addToOopHolder(VMThread *thread, ref oop);

        std::atomic_size_t allocatedOopCount {0};
        std::atomic_size_t allocatedOopMemory {0};

#ifdef DEBUG
        std::unordered_set<OopHolder *> holders;
        SpinLock ttlock;
        std::map<ref, cstring> ttDesc;
#endif
    };
}

#endif