#ifndef OOP_HPP
#define OOP_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "config.hpp"
#include "basic_type.hpp"
#include "composite_ptr.hpp"
#include "utils/spin_lock.hpp"

namespace RexVM {

    struct Class;
    struct InstanceClass;
    struct ArrayClass;
    struct TypeArrayClass;
    struct ObjArrayClass;
    struct OopManager;
    struct VMThread;
    struct SpinLock;
    struct Method;
    struct Field;


    struct OopMonitor {
        std::recursive_mutex monitorMtx;
        std::condition_variable_any monitorCv;
    };

    class Oop {
        static SpinLock monitorLock;
    private:

        //class, dataLength
        Composite<Class *, size_t> comClass;

        //volatile ?, oopMonitorPtr, flags
        Composite<OopMonitor *, u2> comFlags{};

        [[nodiscard]] OopMonitor *getMonitor() const;

    public:

        bool traceMarked{false};

        bool isMirror{false};

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

        void markTraced();

        void clearTraced();

        bool isMarkTraced();

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

        [[nodiscard]] InstanceOop *clone(InstanceOop *newInstance) const;

        [[nodiscard]] InstanceClass *getInstanceClass() const;

    };


    struct MirOop : InstanceOop {
        Composite<voidPtr, u2> mirror;
        #ifdef DEBUG
        cstring mirrorName;
        #endif

        explicit MirOop(InstanceClass *klass, voidPtr mirror, MirrorObjectTypeEnum type);
        ~MirOop();

        void destory();

        [[nodiscard]] MirrorObjectTypeEnum getMirrorObjectType() const;

        [[nodiscard]] Class *getMirrorClass() const;
        [[nodiscard]] Method *getMirrorMethod() const;
        [[nodiscard]] Field *getMirrorField() const;
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
