#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <unordered_set>
#include <vector>
#include <atomic>
#include "config.hpp"
#include "class.hpp"
#include "oop.hpp"

namespace RexVM {

    struct VM;
    struct VMThread;

    extern std::atomic_int oopCount;

    struct OopHolder {
        std::vector<ref> oops;

        explicit OopHolder(size_t size);

        explicit OopHolder();

        void addOop(ref oop);

        void clear();
    };

    struct OopManager {

        VM &vm;
        OopHolder defaultOopHolder{};

        explicit OopManager(VM &vm);
        std::unordered_set<Oop *> allocatedOop;

        [[nodiscard]] InstanceOop *newInstance(VMThread *thread, InstanceClass * klass);
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


        VMThread *newVMThread(VMThread *thread, InstanceClass * klass);

        void addToOopHolder(VMThread *thread, ref oop);

    };

    void traceOop(Oop * root, std::unordered_set<Oop *> &tracedOop);
    void traceInstanceOopChild(InstanceOop * oop, std::unordered_set<Oop *> &tracedOop);
    void traceObjArrayOopChild(ObjArrayOop * oop, std::unordered_set<Oop *> &tracedOop);

    void collectAll(VM &vm);
}

#endif