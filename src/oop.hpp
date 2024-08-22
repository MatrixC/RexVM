#ifndef OOP_HPP
#define OOP_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <tuple>
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

    constexpr u2 TRACED_MASK = 0x8000;     //1000000000000000
    constexpr u2 MIRROR_MASK = 0x4000;     //0100000000000000
    constexpr u2 FINALIZED_MASK = 0x2000;  //0010000000000000

    class Oop {
        static SpinLock monitorLock;
    private:
        //classPtr, dataLength
        Composite<Class *, size_t> comClass{};

        //low [oopMonitor(48), traced(1) isMirror (1) finalized(1) ...(3) {hasHash(1) hash(9)}] high
        Composite<OopMonitor *, u2> comFlags{};

#ifdef DEBUG
        cstring className{};
#endif

        [[nodiscard]] OopMonitor *getMonitor() const;

    protected:
        [[nodiscard]] u2 getFlags() const;
        void setFlags(u2 flags);

    public:
        explicit Oop(Class *klass, size_t dataLength);
        ~Oop();
        [[nodiscard]] bool isInstanceOf(Class *checkClass) const;

        [[nodiscard]] Class *getClass() const;
        [[nodiscard]] size_t getDataLength() const;
        [[nodiscard]] OopTypeEnum getType() const;
        [[nodiscard]] OopMonitor *getAndInitMonitor();

        void lock();
        void unlock();

        void wait(VMThread &currentThread, size_t timeout);
        void notify_one();
        void notify_all();

        void markTraced();
        void clearTraced();
        void setFinalized(bool finalized);
        [[nodiscard]] bool isTraced() const;
        [[nodiscard]] bool isMirror() const;
        [[nodiscard]] bool isFinalized() const;

        void setStringHash(u2 hashCode);
        [[nodiscard]] std::tuple<bool, u2> getStringHash() const;

        [[nodiscard]] size_t getMemorySize() const;
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
