#ifndef OOP_HPP
#define OOP_HPP

#include <memory>
#include <atomic>
#include "config.hpp"
#include "basic_type.hpp"
#include "class.hpp"

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct TypeArrayClass;
    struct ObjArrayClass;
    struct Thread;
    struct OopManager;

    enum class OopTypeEnum {
        InstanceOop,
        TypeArrayOop,
        ObjArrayOop,
    };

    struct Oop {
        const OopTypeEnum type;
        Class *klass;
        bool notGC{false};

        explicit Oop(OopTypeEnum type, Class *klass);

        [[nodiscard]] bool isInstanceOf(Class *checkClass) const;

        void setNotGC();
        [[nodiscard]] bool getNotGC() const;

        virtual ~Oop();

    };

    struct InstanceOop : Oop {
        size_t dataLength;
        std::unique_ptr<Slot[]> data;

        explicit InstanceOop(InstanceClass *klass, size_t dataLength);
        ~InstanceOop() override;

        void setFieldValue(size_t index, Slot value) const;
        [[nodiscard]] Slot getFieldValue(size_t index) const;

        void setFieldValue(const cstring &name, const cstring &descriptor, Slot value);
        [[nodiscard]] Slot getFieldValue(const cstring &name, const cstring &descriptor) const;

        [[nodiscard]] InstanceOop *clone(OopManager &oopManager) const;

    };

    struct MirrorOop : InstanceOop {
        Class *mirrorClass;

        explicit MirrorOop(InstanceClass *klass, Class *mirrorClass);
    };

    struct ThreadOop : InstanceOop {
        Thread *thread;

        explicit ThreadOop(InstanceClass *klass, Thread *thread);
    };

    struct ArrayOop : Oop {
        size_t dataLength;
        
        explicit ArrayOop(OopTypeEnum type, ArrayClass *klass, size_t dataLength);
    };

    struct TypeArrayOop : ArrayOop {
        explicit TypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct ObjArrayOop : ArrayOop {
        std::unique_ptr<ref[]> data;
        explicit ObjArrayOop(ObjArrayClass *klass, size_t dataLength);

    };

    //BooleanTypeArrayOop same as ByteTypeArrayOop
    struct ByteTypeArrayOop : TypeArrayOop {
        std::unique_ptr<u1[]> data;
        explicit ByteTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct ShortTypeArrayOop : TypeArrayOop {
        std::unique_ptr<i2[]> data;
        explicit ShortTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct IntTypeArrayOop : TypeArrayOop {
        std::unique_ptr<i4[]> data;
        explicit IntTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct LongTypeArrayOop : TypeArrayOop {
        std::unique_ptr<i8[]> data;
        explicit LongTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct CharTypeArrayOop : TypeArrayOop {
        std::unique_ptr<cchar_16[]> data;
        explicit CharTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct FloatTypeArrayOop : TypeArrayOop {
        std::unique_ptr<f4[]> data;
        explicit FloatTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };

    struct DoubleTypeArrayOop : TypeArrayOop {
        std::unique_ptr<f8[]> data;
        explicit DoubleTypeArrayOop(TypeArrayClass *klass, size_t dataLength);
    };


}

#endif
