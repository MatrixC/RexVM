#include "java_lang_class.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../class_loader.hpp"
#include "../constant_pool.hpp"
#include "../utils/class_utils.hpp"
#include "../vm.hpp"
#include "../memory.hpp"
#include "../utils/string_utils.hpp"

namespace RexVM::Native {

    void getPrimitiveClass(Frame &frame) {
        auto nameOop = frame.getLocalRef(0);
        const auto name = getStringNativeValue(static_cast<Oop *>(nameOop));
        const auto klass = frame.classLoader.getClass(name);
        const auto mirrorClass = klass->getMirrorOop();
        frame.returnRef(mirrorClass);
    }

    void desiredAssertionStatus0(Frame &frame) {
        frame.returnI4(0);
    }

    void getName0(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        auto strOop = frame.vm.stringPool->getInternString(mirrorClass->name);
        frame.returnRef(strOop);
    }

    void getComponentType(Frame &frame) {
        const auto instance = static_cast<MirrorOop *>(frame.getLocalRef(0));
        const auto mirrorClass = instance->mirrorClass;
        if (mirrorClass->type != ClassTypeEnum::TypeArrayClass && mirrorClass->type != ClassTypeEnum::ObjArrayClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto arrayClass = dynamic_cast<ArrayClass *>(mirrorClass);
        const auto componentName = arrayClass->getComponentClassName();
        const auto componentMirrorOop = frame.classLoader.getClass(componentName)->getMirrorOop();
        frame.returnRef(componentMirrorOop);
    }

    void forName0(Frame &frame) {
        auto nameOop = frame.getLocalRef(0);
        const auto name = getStringNativeValue(static_cast<Oop *>(nameOop));
        const auto initialize = frame.getLocalI4(1);
        //auto classLoaderOop = static_cast<InstanceOop *>(frame.getLocalRef(2));

        const auto newName = replace(name, ".", "/");
        const auto instanceClass = frame.classLoader.getInstanceClass(newName);
        if (initialize) {
            instanceClass->clinit(frame);
        }
        frame.returnRef(instanceClass->getMirrorOop());
    }

    //Field[] getDeclaredFields0(boolean publicOnly)
    void getDeclaredFields0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto self = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = self->mirrorClass;
        auto &classLoader = frame.classLoader;
        const auto retArrayType = classLoader.getObjectArrayClass("java/lang/reflect/Field");
        const auto fieldClass = classLoader.getInstanceClass("java/lang/reflect/Field");

        if (mirrorClass->type != ClassTypeEnum::InstanceClass) {
            const auto returnValue = oopManager->newObjArrayOop(retArrayType, 0);
            frame.returnRef(returnValue);
            return;
        }

        const auto instanceMirrorClass = dynamic_cast<InstanceClass *>(mirrorClass);
        const auto &fields = instanceMirrorClass->fields;
        std::vector<InstanceOop *> fieldInstances;
        for (const auto &field : fields) {
            if (publicOnly && !field->isPublic()) {
                continue;
            }

            

            auto mirrorFieldInstance = oopManager->newInstance(fieldClass);
            mirrorFieldInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(self));
            mirrorFieldInstance->setFieldValue("name", "Ljava/lang/String;",
                                               Slot(frame.vm.stringPool->getInternString(field->name)));
            mirrorFieldInstance->setFieldValue("type", "Ljava/lang/Class;", Slot(field->getTypeClass()->getMirrorOop()));
            mirrorFieldInstance->setFieldValue("modifiers", "I", Slot(field->getModifier()));
            mirrorFieldInstance->setFieldValue("slot", "I", Slot(field->slotId));
            mirrorFieldInstance->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.vm.stringPool->getInternString(field->signature)));
            if (field->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = oopManager->newByteArrayOop(field->runtimeVisibleAnnotationLength, field->runtimeVisibleAnnotation.get());
                mirrorFieldInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }
            fieldInstances.emplace_back(mirrorFieldInstance);
        }

        const auto retArrayOop = oopManager->newObjArrayOop(retArrayType, fieldInstances.size());
        for (auto i = 0; i < fieldInstances.size(); ++i) {
            retArrayOop->data[i] = fieldInstances.at(i);
        }

        frame.returnRef(retArrayOop);
    }
    
    //Constructor<T>[] getDeclaredConstructors0(boolean publicOnly);
    void getDeclaredConstructors0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto self = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = self->mirrorClass;
        auto &classLoader = frame.classLoader;
        const auto retArrayType = classLoader.getObjectArrayClass("java/lang/reflect/Constructor");
        const auto classArrayClass = classLoader.getObjectArrayClass("java/lang/Class");
        const auto constructorClass = classLoader.getInstanceClass("java/lang/reflect/Constructor");

        const auto instanceMirrorClass = dynamic_cast<InstanceClass *>(mirrorClass);
        const auto &methods = instanceMirrorClass->methods;
        std::vector<InstanceOop *> constructorInstances;
        for (auto i = 0; i < methods.size(); ++i) {
            const auto &method = methods.at(i);
            if (method->name != "<init>") {
                continue;
            }

            if (publicOnly && !method->isPublic()) {
                continue;
            }

            const auto paramClasses = method->getParamClasses();
            const auto paramClassesArray = oopManager->newObjArrayOop(classArrayClass, paramClasses.size());
            for (auto j = 0; j < paramClasses.size(); ++j) {
                paramClassesArray->data[j] = paramClasses.at(j);
            }

            

            auto mirrorConstructorInstance = oopManager->newInstance(constructorClass);
            mirrorConstructorInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(self));
            mirrorConstructorInstance->setFieldValue("parameterTypes", "[Ljava/lang/Class;", Slot(paramClassesArray));
            ////mirrorConstructorInstance->setFieldValue("exceptionTypes", "Ljava/lang/Class;", Slot(mirrorClass));
            mirrorConstructorInstance->setFieldValue("modifiers", "I", Slot(method->getModifier()));
            mirrorConstructorInstance->setFieldValue("slot", "I", Slot(i));
            mirrorConstructorInstance->setFieldValue("signature", "Ljava/lang/String;",
                                                     Slot(frame.vm.stringPool->getInternString(method->signature)));
            if (method->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        method->runtimeVisibleAnnotationLength, 
                        method->runtimeVisibleAnnotation.get()
                    );
                mirrorConstructorInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }

            if (method->runtimeVisibleParameterAnnotation != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        method->runtimeVisibleParameterAnnotationLength, 
                        method->runtimeVisibleParameterAnnotation.get()
                    );
                mirrorConstructorInstance->setFieldValue("parameterAnnotations", "[B", Slot(byteArrayOop));
            }

            constructorInstances.emplace_back(mirrorConstructorInstance);
        }

        const auto retArrayOop = oopManager->newObjArrayOop(retArrayType, constructorInstances.size());
        for (auto j = 0; j < constructorInstances.size(); ++j) {
            retArrayOop->data[j] = constructorInstances.at(j);
        }

        frame.returnRef(retArrayOop);
    }

    void isPrimitive(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnBoolean(mirrorClass->type == ClassTypeEnum::PrimitiveClass);
    }

    void isInterface(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnBoolean(mirrorClass->isInterface());
    }

    void isArray(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnBoolean(mirrorClass->isArray());
    }

    void isAssignableFrom(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;

        const auto thatClass = static_cast<MirrorOop *>(frame.getLocalRef(1));
        const auto mirrorThatClass = thatClass->mirrorClass;

        frame.returnBoolean(mirrorClass->isAssignableFrom(mirrorThatClass));
    }

    void getModifiers(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnI4(static_cast<i4>(mirrorClass->accessFlags));
    }

    void getSuperclass(Frame &frame) {
        const auto instance = dynamic_cast<MirrorOop *>(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        const auto superClass = mirrorClass->superClass;
        if (superClass == nullptr) {
            frame.returnRef(nullptr);
        } else {
            frame.returnRef(superClass->getMirrorOop());
        }
    }


}