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
        [[nodiscard]] TypeArrayOop *newTypeArrayOop(BasicType type, size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length);
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length, const u1 *initBuffer);
        [[nodiscard]] CharTypeArrayOop *newCharArrayOop(size_t length);

        ThreadOop *newThreadOop(Thread *thread);
        VMThread *newVMThread();

    };

    void traceOop(Oop * root, std::unordered_set<Oop *> &tracedOop);
    void traceInstanceOopChild(InstanceOop * oop, std::unordered_set<Oop *> &tracedOop);
    void traceObjArrayOopChild(ObjArrayOop * oop, std::unordered_set<Oop *> &tracedOop);

    void gc(VM &vm);
    void gc2(VM &vm);
}

#endif