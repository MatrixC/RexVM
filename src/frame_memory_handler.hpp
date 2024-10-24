#ifndef FRAME_MEMORY_HANDLER_HPP
#define FRAME_MEMORY_HANDLER_HPP
#include "config.hpp"
#include "basic_java_class.hpp"
#include <hash_table8.hpp>

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
    struct ClassMember;
    struct Field;
    struct Method;

    struct ExecuteVirtualMethodCache {
        explicit ExecuteVirtualMethodCache() = default;
        Method *mhMethod{nullptr};
        cview methodName{};
        cview methodDescriptor{};
        u2 mhMethodPopSize{};
        u2 paramSlotSize{};
    };

    struct FrameMemoryHandler {
        explicit FrameMemoryHandler(Frame &frame);

        Frame &frame;
        VMThread &vmThread;
        OopManager &oopManager;
        StringPool &stringPool;
        ClassLoader &classLoader;

        [[nodiscard]] InstanceOop *newInstance(InstanceClass * klass) const;
        [[nodiscard]] MirOop *newMirror(InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type) const;
        [[nodiscard]] ObjArrayOop *newObjArrayOop(ObjArrayClass * klass, size_t length) const;

        [[nodiscard]] ObjArrayOop *newObjectObjArrayOop(size_t length) const;
        [[nodiscard]] ObjArrayOop *newClassObjArrayOop(size_t length) const;
        [[nodiscard]] ObjArrayOop *newStringObjArrayOop(size_t length) const;

        [[nodiscard]] TypeArrayOop *newTypeArrayOop(BasicType type, size_t length) const;
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length) const;
        [[nodiscard]] ByteTypeArrayOop *newByteArrayOop(size_t length, const u1 *initBuffer) const;
        [[nodiscard]] CharTypeArrayOop *newCharArrayOop(size_t length) const;
        [[nodiscard]] ref newMultiArrayOop(u2 index, i4 *dimLength, i2 dimCount);
        [[nodiscard]] ref newMultiArrayOop(i4 *dimLength, i2 dimCount, cview name, i4 currentDim);

        [[nodiscard]] InstanceOop *newBooleanOop(i4 value) const;
        [[nodiscard]] InstanceOop *newByteOop(i4 value) const;
        [[nodiscard]] InstanceOop *newCharOop(i4 value) const;
        [[nodiscard]] InstanceOop *newShortOop(i4 value) const;
        [[nodiscard]] InstanceOop *newIntegerOop(i4 value) const;
        [[nodiscard]] InstanceOop *newFloatOop(f4 value) const;
        [[nodiscard]] InstanceOop *newLongOop(i8 value) const;
        [[nodiscard]] InstanceOop *newDoubleOop(f8 value) const;

        [[nodiscard]] InstanceOop *getInternString(cview str) const;

        [[nodiscard]] TypeArrayClass *getTypeArrayClass(BasicType type) const;
        [[nodiscard]] ObjArrayClass *getObjectArrayClass(const Class &klass) const;
        [[nodiscard]] InstanceClass *loadInstanceClass(const u1 *ptr, size_t length, bool notAnonymous) const;
        [[nodiscard]] InstanceClass *getBasicJavaClass(BasicJavaClassEnum classEnum) const;

        [[nodiscard]] Class *getClass(cview name) const;
        [[nodiscard]] InstanceClass *getInstanceClass(cview name) const;
        [[nodiscard]] ArrayClass *getArrayClass(cview name) const;


        //execute cache

        emhash8::HashMap<u8, voidPtr> executeClassMemberCache{64};
        std::vector<std::unique_ptr<ExecuteVirtualMethodCache>> cacheVector{};

        [[nodiscard]] Field *getRefField(u2 index, bool isStatic);
        [[nodiscard]] Method *getRefMethod(u2 index, bool isStatic);
        [[nodiscard]] Class *getRefClass(u2 index);
        
        [[nodiscard]] ExecuteVirtualMethodCache *resolveInvokeVirtualIndex(u2 index, bool checkMethodHandle);

        [[nodiscard]] Method *linkVirtualMethod(u2 index,
                                                cview methodName,
                                                cview methodDescriptor,
                                                InstanceClass *instanceClass
        );

        [[nodiscard]] InstanceOop *invokeDynamic(u2 invokeDynamicIdx) const;

    };

}

#endif