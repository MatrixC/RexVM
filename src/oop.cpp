#include "oop.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "memory.hpp"
#include "thread.hpp"
#include <algorithm>

namespace RexVM {

    SpinLock Oop::monitorLock;

    Oop::Oop(Class *klass, size_t dataLength) :
            comClass(klass, dataLength) {
        if (dataLength >= 65535) {
            panic("not support");
        }
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
        if (getMonitor() == nullptr) [[unlikely]] {
            Oop::monitorLock.lock();
            if (getMonitor() == nullptr) {
                comFlags.reset(new OopMonitor, 0);
            }
            Oop::monitorLock.unlock();
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
        traceMarked = true;
    }

    void Oop::clearTraced() {
        // if (getClass()->name == "java/lang/Class") {
        //     int i = 10;
        //     const auto mirror = CAST_MIRROR_OOP(this);
        //     if (mirror->getMirrorClass()->name == "java/lang/Long") {
        //         cprintln("Long mark clearn");
        //     }
        // }
        traceMarked = false;
    }

    bool Oop::isMarkTraced() {
        return traceMarked;
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

    MirOop::MirOop(InstanceClass *klass, voidPtr mirrorObj, MirrorObjectTypeEnum type) :
        InstanceOop(klass, klass->instanceSlotCount),
        mirror(mirrorObj, static_cast<u2>(type)) {
        isMirror = true;
    }

    void MirOop::destory() {
        const auto type = getMirrorObjectType();
        switch (type) {
            case MirrorObjectTypeEnum::CLASS: {
                const auto mirrorClass = getMirrorClass();
                mirrorClass->mirrorBase.clear(this);
                break;
            }

            case MirrorObjectTypeEnum::FIELD:
            case MirrorObjectTypeEnum::METHOD:
            case MirrorObjectTypeEnum::CONSTRUCTOR: {
                const auto member = CAST_CLASS_MEMBER(mirror.getPtr());
                member->mirrorBase.clear(this);
                break;
            }

            case MirrorObjectTypeEnum::CONSTANT_POOL: {
                const auto classMirrorOop = CAST_MIRROR_OOP(mirror.getPtr());
                if (classMirrorOop != nullptr) {
                    const auto reKlass = CAST_INSTANCE_CLASS(classMirrorOop->getMirrorClass());
                    reKlass->constantPoolMirrorBase.clear(this);
                }
            }

            default:
                break;
        }
    }

    MirOop::~MirOop() {
        destory();
    }

    MirrorObjectTypeEnum MirOop::getMirrorObjectType() const {
        return static_cast<MirrorObjectTypeEnum>(mirror.getData());
    }

    Class *MirOop::getMirrorClass() const {
        return CAST_CLASS(mirror.getPtr());
    }

    Method *MirOop::getMirrorMethod() const {
        return CAST_METHOD(mirror.getPtr());
    }

    Field *MirOop::getMirrorField() const {
        return CAST_FIELD(mirror.getPtr());
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