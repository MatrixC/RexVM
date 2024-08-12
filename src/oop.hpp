#ifndef OOP_HPP
#define OOP_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "config.hpp"
#include "basic_type.hpp"
#include "class.hpp"
#include "utils/spin_lock.hpp"

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct TypeArrayClass;
    struct ObjArrayClass;
    struct OopManager;
    struct VMThread;

    enum class OopTypeEnum : u1 {
        INSTANCE_OOP = 0,
        TYPE_ARRAY_OOP = 1,
        OBJ_ARRAY_OOP = 2,
    };

    struct OopMonitor {
        std::recursive_mutex monitorMtx;
        std::condition_variable_any monitorCv;
    };

    struct Oop {
        static SpinLock monitorLock;
    private:
        //finalize可直接根据有没有实现对应函数而初始化
        //data部分可改用32bit的Slot 进一步减少占用 或依然使用64slot 但进行一次field slotId重计算 在取数时做处理
        //data指针考虑使用弹性数组


        //low[ classPtr(48) dataLength(16) ]high
        ptr_size mark1{0};
        //low[ mutexPtr(48) finalize(1) ]high
        volatile ptr_size mark2{0};

        [[nodiscard]] OopMonitor *getMonitor() const;

    public:
        explicit Oop(Class *klass, size_t dataLength);

        ~Oop();

        [[nodiscard]] Class *getClass() const;

        [[nodiscard]] size_t getDataLength() const;

        [[nodiscard]] OopTypeEnum getType() const;

        [[nodiscard]] OopMonitor *getAndInitMonitor();

        [[nodiscard]] bool isInstanceOf(Class *checkClass) const;

        void lock();

        void unlock();

        void wait(VMThread &currentThread, size_t timeout);

        void notify_one();

        void notify_all();

    };

    struct InstanceOop : Oop {
        std::unique_ptr<Slot[]> data;

        explicit InstanceOop(InstanceClass *klass, size_t dataLength);

        explicit InstanceOop(InstanceClass *klass);

        ~InstanceOop();

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
