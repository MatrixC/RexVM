#ifndef OOP_HPP
#define OOP_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "config.hpp"
#include "basic_type.hpp"
#include "class.hpp"

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct TypeArrayClass;
    struct ObjArrayClass;
    struct OopManager;

    enum class OopTypeEnum {
        INSTANCE_OOP,
        TYPE_ARRAY_OOP,
        OBJ_ARRAY_OOP,
    };

    struct Oop {
        //finalize可直接根据有没有实现对应函数而初始化
        //markword1: low[ classPtr(48) dataLength(16) ]high
        //markword2: low[ mutexPtr(48) remain(14) finalize(1) type(2) waitFlag(1) ]high
        //data部分可改用32bit的Slot 进一步减少占用 或依然使用64slot 但进行一次field slotId重计算 在取数时做处理
        //data指针考虑使用弹性数组

        const OopTypeEnum type;
        Class *klass;

        bool waitFlag{false};
        std::recursive_mutex monitorMtx;
        std::condition_variable_any monitorCv;

        explicit Oop(OopTypeEnum type, Class *klass);

        [[nodiscard]] bool isInstanceOf(Class *checkClass) const;

        virtual ~Oop();

    };

    struct InstanceOop : Oop {
        size_t dataLength;
        std::unique_ptr<Slot[]> data;

        explicit InstanceOop(InstanceClass *klass, size_t dataLength);
        explicit InstanceOop(InstanceClass *klass);
        ~InstanceOop() override;

        void setFieldValue(size_t index, Slot value) const;
        [[nodiscard]] Slot getFieldValue(size_t index) const;

        void setFieldValue(const cstring &name, const cstring &descriptor, Slot value) const;
        [[nodiscard]] Slot getFieldValue(const cstring &name, const cstring &descriptor) const;

        [[nodiscard]] InstanceOop *clone(OopManager &oopManager) const;

        [[nodiscard]] InstanceClass *getInstanceClass() const;

    };

    struct MirrorOop : InstanceOop {
        //对于MirrorOop或许可以直接用InstanceOop，在最后的加一两个slot里存入自己的数据
        Class *mirrorClass;
        std::unique_ptr<InstanceOop> constantPoolOop;

        explicit MirrorOop(InstanceClass *klass, Class *mirrorClass);
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
