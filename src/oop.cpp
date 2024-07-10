#include "oop.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "memory.hpp"
#include "utils/format.hpp"
#include <algorithm>

namespace RexVM {

    Oop::Oop(const OopTypeEnum type, Class *klass) : type(type), klass(klass) {
    }

    bool Oop::isInstanceOf(Class *checkClass) const {
        return checkClass->isAssignableFrom(this->klass);
    }

    Oop::~Oop() = default;

    void initInstanceField(const InstanceOop *oop, InstanceClass *klass) {
        for (const auto &field: klass->fields) {
            if (!field->isStatic()) {
                const auto slotType = field->getFieldSlotType();
                const auto slotId = field->slotId;
                switch (slotType) {
                    case SlotTypeEnum::I4:
                        oop->data[slotId] = Slot(static_cast<i4>(0));
                        break;

                    case SlotTypeEnum::REF:
                        oop->data[slotId] = Slot(nullptr);
                        break;

                    case SlotTypeEnum::F4:
                        oop->data[slotId] = Slot(static_cast<f4>(0));
                        break;

                    case SlotTypeEnum::I8:
                        oop->data[slotId] = Slot(static_cast<i8>(0));
                        break;

                    case SlotTypeEnum::F8:
                        oop->data[slotId] = Slot(static_cast<f4>(0));
                        break;

                    default:
                        panic("initInstanceField error");
                }
            }
        }

        if (klass->superClass != nullptr) {
            initInstanceField(oop, klass->superClass);
        }
    }

    InstanceOop::InstanceOop(InstanceClass *klass, const size_t dataLength) :
            Oop(OopTypeEnum::InstanceOop, klass), 
            dataLength(dataLength), 
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

    void InstanceOop::setFieldValue(const cstring &name, const cstring &descriptor, Slot value) {
        auto instanceClass = getInstanceClass();
        auto field = instanceClass->getField(name, descriptor, false);
        data[field->slotId] = value;
    }

    Slot InstanceOop::getFieldValue(const cstring &name, const cstring &descriptor) const {
        auto instanceClass = getInstanceClass();
        auto field = instanceClass->getField(name, descriptor, false);
        return data[field->slotId];
    }

    InstanceOop *InstanceOop::clone(OopManager &manager) const {
        auto newInstance = manager.newInstance(getInstanceClass());
        const auto from = this->data.get();
        const auto to = newInstance->data.get();
        std::copy(from, from + dataLength, to);
        return newInstance;
    }

    InstanceClass *InstanceOop::getInstanceClass() const {
        return static_cast<InstanceClass *>(klass);
    }

    MirrorOop::MirrorOop(InstanceClass *klass, Class *mirrorClass) :
        InstanceOop(klass, klass->instanceSlotCount),
        mirrorClass(mirrorClass) {
    }

    ArrayOop::ArrayOop(const OopTypeEnum type, ArrayClass *klass, const size_t dataLength) :
            Oop(type, klass), dataLength(dataLength) {
    }

    TypeArrayOop::TypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
            ArrayOop(OopTypeEnum::TypeArrayOop, klass, dataLength) {
    }

    ObjArrayOop::ObjArrayOop(ObjArrayClass *klass, const size_t dataLength) :
            ArrayOop(OopTypeEnum::ObjArrayOop, klass, dataLength), data(std::make_unique<ref[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, nullptr);
    }

    ByteTypeArrayOop::ByteTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<u1[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<u1>(0));
    }

    ShortTypeArrayOop::ShortTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i2[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<i2>(0));
    }

    IntTypeArrayOop::IntTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i4[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<i4>(0));
    }

    LongTypeArrayOop::LongTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<i8[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<i8>(0));
    }

    CharTypeArrayOop::CharTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<cchar_16[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, '\0');
    }

    FloatTypeArrayOop::FloatTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<f4[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<f4>(0));
    }

    DoubleTypeArrayOop::DoubleTypeArrayOop(TypeArrayClass *klass, const size_t dataLength) :
        TypeArrayOop(klass, dataLength), data(std::make_unique<f8[]>(dataLength)) {
            std::fill_n(data.get(), dataLength, static_cast<f8>(0));
    }


}