#include "mirror_base.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "mirror_oop.hpp"
#include "frame.hpp"
#include "constant_info.hpp"
#include "utils/spin_lock.hpp"

namespace RexVM {

    MirOop *MirrorBase::getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, voidPtr mirrorObj, SpinLock &lock, bool init) {
        if (mirOop == nullptr) [[unlikely]] {
            if (!init) [[unlikely]] {
                return nullptr;
            }
            {
                std::lock_guard<SpinLock> guard(lock);
                if (mirOop == nullptr && frame != nullptr) {
                    mirOop = frame->mem.newMirror(nullptr, mirrorObj, type);
                    switch (type) {
                        case MirrorObjectTypeEnum::CLASS:
                            initClassMirrorOop(*frame, CAST_CLASS(mirrorObj));
                            break;

                        case MirrorObjectTypeEnum::FIELD:
                            initFieldMirrorOop(*frame, CAST_FIELD(mirrorObj));
                            break;
                        
                        case MirrorObjectTypeEnum::METHOD:
                            initMethodMirrorOop(*frame, CAST_METHOD(mirrorObj), false);
                            break;

                        case MirrorObjectTypeEnum::CONSTRUCTOR:
                            initMethodMirrorOop(*frame, CAST_METHOD(mirrorObj), true);
                            break;

                        case MirrorObjectTypeEnum::CONSTANT_POOL:
                            initConstantPoolMirrorOop(*frame, CAST_CLASS(mirrorObj));
                            break;

                        default:
                            break;
                    }
                }
            }
        }
        return mirOop;
    }

    void MirrorBase::clear(voidPtr checkPtr) {
#ifdef DEBUG
        if (checkPtr != mirOop) {
            panic("clear check error");
        }
#endif
        mirOop = nullptr;
    }

    void MirrorBase::initClassMirrorOop(Frame &frame, Class *klass) const {
        #ifdef DEBUG
        mirOop->mirrorName = klass->name;
        #endif
    }

    void MirrorBase::initMethodMirrorOop(Frame &frame, Method *method, bool isConstructor) const {
        auto &klass = method->klass;
        const auto paramClasses = method->getParamClasses();
        const auto paramClassesArrayOop = frame.mem.newClassObjArrayOop(paramClasses.size());
        FOR_FROM_ZERO(paramClasses.size()) {
            paramClassesArrayOop->data[i] = paramClasses[i]->getMirror(&frame);
        }

        const auto exceptionsSize = CAST_SIZE_T(method->exceptionsIndex.data.getData());
        const auto exceptionArrayOop = frame.mem.newClassObjArrayOop(exceptionsSize);
        FOR_FROM_ZERO(exceptionsSize) {
            const auto exceptionIdx = method->exceptionsIndex.data.getPtr()[i];
            const auto exceptionClassName = 
                getConstantStringFromPoolByIndexInfo(klass.constantPool, exceptionIdx);
            exceptionArrayOop->data[i] = frame.mem.getClass(exceptionClassName)->getMirror(&frame);
        }

        mirOop->setFieldValue("override", "Z", Slot(CAST_I4(0)));
        mirOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(klass.getMirror(&frame)));
        mirOop->setFieldValue("slot", "I", Slot(method->slotId));
        mirOop->setFieldValue("parameterTypes", "[Ljava/lang/Class;", Slot(paramClassesArrayOop));
        mirOop->setFieldValue("exceptionTypes", "[Ljava/lang/Class;", Slot(exceptionArrayOop));
        mirOop->setFieldValue("modifiers", "I", Slot(method->getModifier()));
        if (!method->signature.empty()) {
            mirOop->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.mem.getInternString(method->signature)));
        }
        
        if (!isConstructor) {
            mirOop->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(method->name)));
            mirOop->setFieldValue("returnType", "Ljava/lang/Class;", Slot(frame.mem.getClass(method->returnType)->getMirror(&frame)));
        }

        if (method->basicAnnotationContainer != nullptr) {
            const auto byteArrayOop = method->basicAnnotationContainer->runtimeVisibleAnnotation.createByteTypeArrayOop(frame);
            if (byteArrayOop != nullptr) {
                mirOop->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }
        }

        if (method->methodAnnotationContainer != nullptr) {
            {
                const auto byteArrayOop = 
                    method->methodAnnotationContainer->runtimeVisibleParameterAnnotation.createByteTypeArrayOop(frame);
                if (byteArrayOop != nullptr) {
                    mirOop->setFieldValue("parameterAnnotations", "[B", Slot(byteArrayOop));
                }
            }

            if (!isConstructor) {
                const auto byteArrayOop = 
                    method->methodAnnotationContainer->annotationDefault.createByteTypeArrayOop(frame);
                if (byteArrayOop != nullptr) {
                    mirOop->setFieldValue("annotationDefault", "[B", Slot(byteArrayOop));
                }
            }
        }
        
        #ifdef DEBUG
        mirOop->mirrorName = klass.name + "." + method->name;
        #endif
    }

    void MirrorBase::initFieldMirrorOop(Frame &frame, Field *field) const {
        auto &klass = field->klass;
        mirOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(klass.getMirror(&frame)));
        mirOop->setFieldValue("slot", "I", Slot(field->slotId));
        mirOop->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->name)));
        mirOop->setFieldValue("type", "Ljava/lang/Class;", Slot(field->getTypeClass()->getMirror(&frame)));
        mirOop->setFieldValue("modifiers", "I", Slot(field->getModifier()));
        mirOop->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->signature)));

        if (field->basicAnnotationContainer != nullptr) {
            const auto byteArrayOop = field->basicAnnotationContainer->runtimeVisibleAnnotation.createByteTypeArrayOop(frame);
            if (byteArrayOop != nullptr) {
                mirOop->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }
        }

        #ifdef DEBUG
        mirOop->mirrorName = klass.name + "." + field->name;
        #endif

    }

    void MirrorBase::initConstantPoolMirrorOop(Frame &frame, Class *klass) const {
    }

    MirrorBase::MirrorBase() = default;

    MirrorBase::~MirrorBase() {
        //这里不应该删除 应该交给gc去清理
        // delete mirOop;
        // mirOop = nullptr;
    }

}