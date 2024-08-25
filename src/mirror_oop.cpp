#include "mirror_oop.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "method_handle.hpp"

namespace RexVM {

    MirOop::MirOop(InstanceClass *klass, voidPtr mirrorObj, MirrorObjectTypeEnum type) :
            InstanceOop(klass, klass->instanceSlotCount),
            mirror(mirrorObj, static_cast<u2>(type)) {
        setFlags(getFlags() | MIRROR_MASK);
    }

    void MirOop::clearHolder() {
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
                const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass());
                mirrorClass->constantPoolMirrorBase.clear(this);
            }

            default:
                break;
        }
    }

    MirOop::~MirOop() {
        clearHolder();
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

    Method *MirOop::getMemberNameMethod() {
        auto methodPtr = CAST_METHOD(mirror.getPtr());
        if (methodPtr == nullptr) {
            auto [klass, name, type, flags, kind, isStatic, descriptor]
                    = methodHandleGetFieldFromMemberName(this);
            methodPtr = klass->getMethod(name, descriptor, isStatic);
            mirror.setPtr(methodPtr);
        }
        return methodPtr;
    }

}