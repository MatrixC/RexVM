#include "oop.hpp"
#include <algorithm>
#include "class.hpp"
#include "class_member.hpp"
#include "thread.hpp"


namespace RexVM {

    Oop::Oop(Class *klass, size_t dataLength) :
            comClass(klass, dataLength),
            comFlags(nullptr, FINALIZED_MASK) {
        //dataLength >= 65535, not support
#ifdef DEBUG
        className = klass->getClassName();
#endif
    }

    bool Oop::isInstanceOf(Class *checkClass) const {
        return checkClass->isAssignableFrom(this->getClass());
    }

    Oop::~Oop() {
        delete getMonitor();
    }

    Class *Oop::getClass() const {
        return comClass.getPtr();
    }

    size_t Oop::getDataLength() const {
        return comClass.getData();
    }

    OopMonitor *Oop::getMonitor() const {
        return comFlags.getPtr();
    }

    u2 Oop::getFlags() const {
        return comFlags.getData();
    }
    void Oop::setFlags(u2 flags) {
        comFlags.setData(flags);
    }

    OopTypeEnum Oop::getType() const {
        switch (getClass()->type) {
            case ClassTypeEnum::INSTANCE_CLASS:
                return OopTypeEnum::INSTANCE_OOP;
            case ClassTypeEnum::OBJ_ARRAY_CLASS:
                return OopTypeEnum::OBJ_ARRAY_OOP;
            case ClassTypeEnum::TYPE_ARRAY_CLASS:
                return OopTypeEnum::TYPE_ARRAY_OOP;
            default:
                panic("error oop");
                return OopTypeEnum::INSTANCE_OOP;
        }
    }

    OopMonitor *Oop::getAndInitMonitor() {
        static SpinLock monitorLock;

        if (getMonitor() == nullptr) [[unlikely]] {
            std::lock_guard<SpinLock> guard(monitorLock);
            if (getMonitor() == nullptr) {
                comFlags.setPtr(new OopMonitor);
            }
        }
        return getMonitor();
    }

    void Oop::lock() {
        getAndInitMonitor()->monitorMtx.lock();
    }

    void Oop::unlock() {
        getAndInitMonitor()->monitorMtx.unlock();
    }

    void Oop::wait(VMThread &currentThread, size_t timeout) {
        const auto selfMonitor = getAndInitMonitor();
        std::unique_lock<std::recursive_mutex> lock(selfMonitor->monitorMtx, std::adopt_lock);
        const auto backupStatus = currentThread.getStatus();
        currentThread.setStatus(ThreadStatusEnum::WAITING);
        if (timeout == 0) [[likely]] {
            selfMonitor->monitorCv.wait(lock);
        } else {
            selfMonitor->monitorCv.wait_for(lock, std::chrono::microseconds(timeout));
        }
        currentThread.setStatus(backupStatus);
    }

    void Oop::notify_one() {
        getAndInitMonitor()->monitorCv.notify_one();
    }

    void Oop::notify_all() {
        getAndInitMonitor()->monitorCv.notify_all();
    }

    void Oop::markTraced() {
        setFlags(getFlags() | TRACED_MASK);
    }

    void Oop::clearTraced() {
        setFlags(getFlags() & (~TRACED_MASK));
    }

    bool Oop::isTraced() const {
        return getFlags() & TRACED_MASK;
    }

    bool Oop::isMirror() const {
        return getFlags() & MIRROR_MASK;
    }

    void Oop::setFinalized(bool finalized) {
        auto flags = getFlags();
        flags = finalized ? (flags | FINALIZED_MASK) : (flags & (~FINALIZED_MASK));
        setFlags(flags);
    }

    bool Oop::isFinalized() const {
        return getFlags() & FINALIZED_MASK;
    }

    //hasHash 1bit, hashCode 9bit
    constexpr u2 HAS_HASH_MASK = 0x200;  //1000000000
    constexpr u2 HASH_MASK = 0x3ff;      //1111111111
    constexpr u2 HASH_VAL_MASK = 0x1ff;  //0111111111
    void Oop::setStringHash(u2 hashCode) {
#ifdef DEBUG
        if (hashCode >= HAS_HASH_MASK) {
            panic("error HashCode");
        }
#endif
        hashCode |= HAS_HASH_MASK; //final hashCode

        const auto flags = (getFlags() & ~HASH_MASK) | hashCode;
        setFlags(flags);
    }
    
    std::tuple<bool, u2> Oop::getStringHash() const {
        const auto hashCode = getFlags() & HASH_MASK;
        const auto hasHash = (hashCode & HAS_HASH_MASK) != 0;
        const auto hashVal = hashCode & HASH_VAL_MASK;
        return std::make_tuple(hasHash, hashVal);
    }

    size_t Oop::getMemorySize() const {
        const auto oopType = getType();
        const auto dataLength = getDataLength();
        switch (oopType) {
            case OopTypeEnum::INSTANCE_OOP:
                return sizeof(InstanceOop) + dataLength * SLOT_BYTE_SIZE;
            case OopTypeEnum::OBJ_ARRAY_OOP:
                return sizeof(ObjArrayOop) + dataLength * SLOT_BYTE_SIZE;
            case OopTypeEnum::TYPE_ARRAY_OOP: {
                const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(getClass());
                const auto elementSize = getElementSizeByBasicType(typeArrayClass->elementType);
                //sizeof(ByteTypeArrayOop) = sizeof(TypeArrayOop) + sizeof(ptr)
                return sizeof(ByteTypeArrayOop) + dataLength * elementSize;
            }
        }
        return 0;
    }

    void initInstanceField(const InstanceOop *oop, InstanceClass *klass) {
        //std::memset(oop->data.get(), 0, sizeof(Slot) * oop->getDataLength());
        for (const auto &field: klass->fields) {
            if (!field->isStatic()) {
                const auto slotType = field->getFieldSlotType();
                const auto slotId = field->slotId;
                switch (slotType) {
                    case SlotTypeEnum::I4:
                        oop->data[slotId] = Slot(CAST_I4(0));
                        break;
                    case SlotTypeEnum::F4:
                        oop->data[slotId] = Slot(CAST_F4(0));
                        break;
                    case SlotTypeEnum::I8:
                        oop->data[slotId] = Slot(CAST_I8(0));
                        break;
                    case SlotTypeEnum::F8:
                        oop->data[slotId] = Slot(CAST_F8(0));
                        break;
                    case SlotTypeEnum::REF:
                        oop->data[slotId] = Slot(nullptr);
                        break;
                    default:
                        panic("initInstanceField error");
                }
            }
        }
        if (klass->getSuperClass() != nullptr) {
            initInstanceField(oop, klass->getSuperClass());
        }
    }

    InstanceOop::InstanceOop(InstanceClass *klass, const size_t dataLength) :
            Oop(klass, dataLength),
            data(std::make_unique<Slot[]>(dataLength)) {
        initInstanceField(this, klass);
        if (klass->overrideFinalize) {
            setFinalized(false);
        }
    }

    InstanceOop::InstanceOop(InstanceClass *klass) : InstanceOop(klass, klass->instanceSlotCount) {
    }

    InstanceOop::~InstanceOop() = default;

    Slot InstanceOop::getFieldValue(size_t index) const {
        return data[index];
    }

    void InstanceOop::setFieldValue(size_t index, Slot value) const {
        data[index] = value;
    }

    void InstanceOop::setFieldValue(const cstring &name, const cstring &descriptor, Slot value) const {
        auto instanceClass = getInstanceClass();
        auto field = instanceClass->getField(name, descriptor, false);
        data[field->slotId] = value;
    }

    Slot InstanceOop::getFieldValue(const cstring &name, const cstring &descriptor) const {
        auto instanceClass = getInstanceClass();
        auto field = instanceClass->getField(name, descriptor, false);
        return data[field->slotId];
    }

    InstanceOop *InstanceOop::clone(InstanceOop *newInstance) const {
        const auto from = this->data.get();
        const auto to = newInstance->data.get();
        std::copy(from, from + getDataLength(), to);
        return newInstance;
    }

    InstanceClass *InstanceOop::getInstanceClass() const {
        return CAST_INSTANCE_CLASS(getClass());
    }

    ArrayOop::ArrayOop(const OopTypeEnum type, ArrayClass *klass, const size_t dataLength) :
            Oop(klass, dataLength) {
    }

    TypeArrayOop::TypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
            ArrayOop(OopTypeEnum::TYPE_ARRAY_OOP, klass, dataLength) {
    }

    ObjArrayOop::ObjArrayOop(ObjArrayClass *klass, const size_t dataLength) :
            ArrayOop(OopTypeEnum::OBJ_ARRAY_OOP, klass, dataLength), data(std::make_unique<ref[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, nullptr);
    }

    ByteTypeArrayOop::ByteTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<u1[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_U1(0));
    }

    ShortTypeArrayOop::ShortTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i2[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_I2(0));
    }

    IntTypeArrayOop::IntTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i4[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_I4(0));
    }

    LongTypeArrayOop::LongTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i8[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_I8(0));
    }

    CharTypeArrayOop::CharTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<cchar_16[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, '\0');
    }

    FloatTypeArrayOop::FloatTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<f4[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_F4(0));
    }

    DoubleTypeArrayOop::DoubleTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<f8[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, CAST_F8(0));
    }

}