#include "mirror_base.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "frame.hpp"
#include "constant_info.hpp"
#include "utils/spin_lock.hpp"

namespace RexVM {

    MirOop *MirrorBase::getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, voidPtr mirrorObj, SpinLock &lock, bool init) {
        if (mirOop == nullptr) [[unlikely]] {
            if (!init) [[unlikely]] {
                return nullptr;
            }
            lock.lock();
            if (mirOop == nullptr) {
                if (frame != nullptr) {
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
            lock.unlock();
        }
        return mirOop;
    }

    void MirrorBase::clear(voidPtr checkPtr) {
        int i = 10;
        //cprintln("clear.....");
        if (checkPtr != mirOop) {
            cprintln("error...");
        }
        mirOop = nullptr;
    }

    void MirrorBase::initClassMirrorOop(Frame &frame, Class *klass) {
        #ifdef DEBUG
        mirOop->mirrorName = klass->name;
        #endif
    }

    void MirrorBase::initMethodMirrorOop(Frame &frame, Method *method, bool isConstructor) {
        auto &klass = method->klass;
        const auto paramClasses = method->getParamClasses();
        const auto paramClassesArrayOop = frame.mem.newClassObjArrayOop(paramClasses.size());
        FOR_FROM_ZERO(paramClasses.size()) {
            paramClassesArrayOop->data[i] = paramClasses.at(i)->getMirror(&frame);
        }

        const auto exceptionArrayOop = frame.mem.newClassObjArrayOop(method->exceptionsIndex.size());
        FOR_FROM_ZERO(method->exceptionsIndex.size()) {
            const auto exceptionIdx = method->exceptionsIndex.at(i);
            const auto exceptionClassName = 
                getConstantStringFromPoolByIndexInfo(klass.constantPool, exceptionIdx);
            exceptionArrayOop->data[i] = frame.mem.getClass(exceptionClassName)->getMirror(&frame);
        }
        mirOop->setFieldValue("override", "Z", Slot(CAST_I4(0)));
        mirOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(klass.getMirror(&frame)));
        mirOop->setFieldValue("slot", "I", Slot(method->index));
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
        
        if (method->runtimeVisibleAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
                    method->runtimeVisibleAnnotationLength, 
                    method->runtimeVisibleAnnotation.get()
                );
            mirOop->setFieldValue("annotations", "[B", Slot(byteArrayOop));
        }
        
        if (method->runtimeVisibleParameterAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
                    method->runtimeVisibleParameterAnnotationLength, 
                    method->runtimeVisibleParameterAnnotation.get()
                );
            mirOop->setFieldValue("parameterAnnotations", "[B", Slot(byteArrayOop));
        }
        
        if (!isConstructor && method->annotationDefault != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
                    method->annotationDefaultLength, 
                    method->annotationDefault.get()
                );
            mirOop->setFieldValue("annotationDefault", "[B", Slot(byteArrayOop));
        }

        #ifdef DEBUG
        mirOop->mirrorName = klass.name + "." + method->name;
        #endif
    }


    void *jjjj = nullptr;
    void MirrorBase::initFieldMirrorOop(Frame &frame, Field *field) {
        auto &klass = field->klass;
        mirOop->setFieldValue("clazz", "Ljava/lang/Class;", Slot(klass.getMirror(&frame)));
        mirOop->setFieldValue("slot", "I", Slot(field->slotId));
        mirOop->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->name)));
        mirOop->setFieldValue("type", "Ljava/lang/Class;", Slot(field->getTypeClass()->getMirror(&frame)));
        mirOop->setFieldValue("modifiers", "I", Slot(field->getModifier()));
        mirOop->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->signature)));
        if (field->runtimeVisibleAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
                    field->runtimeVisibleAnnotationLength, 
                    field->runtimeVisibleAnnotation.get()
                );
            mirOop->setFieldValue("annotations", "[B", Slot(byteArrayOop));
        }

        #ifdef DEBUG
        mirOop->mirrorName = klass.name + "." + field->name;
        #endif

        if (field->name == "¤ #,##0.00;-¤ #,##0.00") {
            jjjj = CAST_VOID_PTR(mirOop);
            cprintln("jjjj annoptr {}", jjjj);
        }
    }

    void MirrorBase::initConstantPoolMirrorOop(Frame &frame, Class *klass) {
        mirOop->setFieldValue("constantPoolOop", "Ljava/lang/Object;", Slot(klass->getMirror(&frame)));
    }

    MirrorBase::MirrorBase() = default;

    MirrorBase::~MirrorBase() {
        //这里不应该删除 应该交给gc去清理
        // delete mirOop;
        // mirOop = nullptr;
    }

}