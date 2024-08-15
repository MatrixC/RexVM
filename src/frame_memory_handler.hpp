#ifndef FRAME_MEMORY_HANDLER_HPP
#define FRAME_MEMORY_HANDLER_HPP
#include "config.hpp"
#include "basic_type.hpp"
#include "basic_java_class.hpp"
#include "oop.hpp"

namespace RexVM {

    struct Frame;
    struct OopManager;
    struct VMThread;

    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct ObjArrayClass;
    struct TypeArrayClass;
    struct InstanceOop;
    struct MirOop;
    struct ObjArrayOop;
    struct TypeArrayOop;
    struct ByteTypeArrayOop;
    struct CharTypeArrayOop;
    struct StringPool;
    struct ClassLoader;
    struct VMThread;

    struct FrameMemoryHandler {
        explicit FrameMemoryHandler(const Frame &frame);

        VMThread &vmThread;
        OopManager &oopManager;
        StringPool &stringPool;
        ClassLoader &classLoader;

        [[nodiscard]] InstanceOop *newInstance(InstanceClass * klass);
        [[nodiscard]] MirOop *newMirror(InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type);
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

        [[nodiscard]] VMThread *newVMThread(InstanceClass * klass);

        [[nodiscard]] InstanceOop *getInternString(const cstring &str);

        [[nodiscard]] Class *getClass(const cstring &name);
        [[nodiscard]] InstanceClass *getInstanceClass(const cstring &name);
        [[nodiscard]] ArrayClass *getArrayClass(const cstring &name);
        [[nodiscard]] TypeArrayClass *getTypeArrayClass(BasicType type);
        [[nodiscard]] ObjArrayClass *getObjectArrayClass(const cstring &name);
        [[nodiscard]] InstanceClass *loadInstanceClass(u1 *ptr, size_t length, bool notAnonymous);
        [[nodiscard]] InstanceClass *getBasicJavaClass(BasicJavaClassEnum classEnum) const;


    };

}

#endif