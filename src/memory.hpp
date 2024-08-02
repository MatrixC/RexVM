#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <unordered_set>
#include "class.hpp"
#include "oop.hpp"
#include "thread.hpp"

namespace RexVM {

    struct VM;

    struct OopManager {

        VM &vm;

        explicit OopManager(VM &vm);
        std::unordered_set<Oop *> allocatedOop;

        [[nodiscard]] InstanceOop *newInstance(InstanceClass * klass);
        [[nodiscard]] ObjArrayOop *newObjArrayOop(ObjArrayClass * klass, size_t length);

        [[nodiscard]] ObjArrayOop *newObjectObjArrayOop(size_t length);
        [[nodiscard]] ObjArrayOop *newClassObjArrayOop(size_t length);
        [[nodiscard]] ObjArrayOop *newStringObjArrayOop(size_t length);

        [[nodiscard]] TypeArrayOop *newTypeArrayOop(BasicType type, size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length, const u1 *initBuffer);
        [[nodiscard]] CharTypeArrayOop *newCharArrayOop(size_t length);

        [[nodiscard]] InstanceOop *newBooleanOop(i4 value);
        [[nodiscard]] InstanceOop *newByteOop(i4 value);
        [[nodiscard]] InstanceOop *newCharOop(i4 value);
        [[nodiscard]] InstanceOop *newShortOop(i4 value);
        [[nodiscard]] InstanceOop *newIntegerOop(i4 value);
        [[nodiscard]] InstanceOop *newFloatOop(f4 value);
        [[nodiscard]] InstanceOop *newLongOop(i8 value);
        [[nodiscard]] InstanceOop *newDoubleOop(f8 value);


        VMThread *newVMThread(InstanceClass * const klass);
        VMThread *newMainVMThread(Method &method, std::vector<Slot> params);

    };

    void traceOop(Oop * root, std::unordered_set<Oop *> &tracedOop);
    void traceInstanceOopChild(InstanceOop * oop, std::unordered_set<Oop *> &tracedOop);
    void traceObjArrayOopChild(ObjArrayOop * oop, std::unordered_set<Oop *> &tracedOop);

    void collectAll(VM &vm);
}

#endif