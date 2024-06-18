#ifndef MEMORY_HPP
#define MEMORY_HPP
#include <unordered_set>
#include "class.hpp"
#include "oop.hpp"

namespace RexVM {

    struct VM;

    struct OopManager {

        VM &vm;

        explicit OopManager(VM &vm);
        std::unordered_set<Oop *> allocatedOop;

        [[nodiscard]] InstanceOop *newInstance(InstanceClass * const klass);
        [[nodiscard]] ObjArrayOop *newObjArrayOop(ObjArrayClass * const klass, size_t length);
        [[nodiscard]] TypeArrayOop *newTypeArrayOop(BasicType type, size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length, u1 *initBuffer);
        [[nodiscard]] CharTypeArrayOop *newCharArrayOop(size_t length);

        ThreadOop *newThreadOop(Thread *thread);

    };

    void traceOop(Oop * const root, std::unordered_set<Oop *> &tracedOop);
    void traceInstanceOopChild(InstanceOop * const oop, std::unordered_set<Oop *> &tracedOop);
    void traceObjArrayOopChild(ObjArrayOop * const oop, std::unordered_set<Oop *> &tracedOop);

    void gc(VM &vm);
    void gc2(VM &vm);
}

#endif