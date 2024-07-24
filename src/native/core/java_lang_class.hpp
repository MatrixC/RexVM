#ifndef NATIVE_CORE_JAVA_LANG_CLASS_HPP
#define NATIVE_CORE_JAVA_LANG_CLASS_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../constant_pool.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/class_utils.hpp"

namespace RexVM::Native::Core {

    Class *getMirrorClass(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThisInstance());
        return instance->mirrorClass;
    }

    InstanceClass *getMirrorInstanceClass(Frame &frame) {
        return CAST_INSTANCE_CLASS(getMirrorClass(frame));
    }

    //native String getName0();
    void getName0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto className = mirrorClass->name;
        const auto javaClassName = getJavaClassName(className);
        auto strOop = frame.vm.stringPool->getInternString(javaClassName);
        frame.returnRef(strOop);
    }

    //static Class<?> forName0(String className, boolean initialize, ClassLoader loader, Class<?> caller)
    void forName0(Frame &frame) {
        auto classNameOop = frame.getLocalRef(0);
        const auto className = StringPool::getJavaString(CAST_INSTANCE_OOP(classNameOop));
        const auto initialize = frame.getLocalI4(1);

        const auto jvmClassName = getJVMClassName(className);
        const auto klass = frame.getCurrentClassLoader()->getClass(jvmClassName);
        if (initialize && klass->type == ClassTypeEnum::InstanceClass) {
            CAST_INSTANCE_CLASS(klass)->clinit(frame);
        }
        frame.returnRef(klass->getMirrorOop());
    }

    //static native Class<?> getPrimitiveClass(String name);
    void getPrimitiveClass(Frame &frame) {
        auto classNameOop = frame.getLocalRef(0);
        const auto className = StringPool::getJavaString(CAST_INSTANCE_OOP(classNameOop));
        const auto klass = frame.getCurrentClassLoader()->getClass(className);
        frame.returnRef(klass->getMirrorOop());
    }

    //static native boolean desiredAssertionStatus0(Class<?> clazz);
    void desiredAssertionStatus0(Frame &frame) {
        frame.returnBoolean(false);
    }

    //native boolean isPrimitive();
    void isPrimitive(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->type == ClassTypeEnum::PrimitiveClass);
    }

    //native boolean isInterface();
    void isInterface(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->isInterface());
    }

    //native boolean isArray();
    void isArray(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->isArray());
    }

    //native boolean isAssignableFrom(Class<?> cls);
    void isAssignableFrom(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto thatClass = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorThatClass = thatClass->mirrorClass;
        frame.returnBoolean(mirrorClass->isAssignableFrom(mirrorThatClass));
    }

    // void isInstance(Frame &frame) {
    //     const auto mirrorClass = getMirrorClass(frame);
    //     const auto objOop = frame.getLocalRef(1);
    //     frame.returnBoolean(objOop->klass->isAssignableFrom(mirrorClass));
    // }

    //native int getModifiers();
    void getModifiers(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto removeSuper = ~CAST_I4(AccessFlagEnum::ACC_SUPER);
        frame.returnI4(CAST_I4(mirrorClass->accessFlags) & removeSuper);
    }

        //native boolean isInstance(Object obj);
    void isInstance(Frame &frame) {
        const auto thisClass = getMirrorClass(frame);
        const auto objOop = frame.getLocalRef(1);
        const auto objClass = objOop->klass;
        const auto result = objClass->isInstanceOf(thisClass);
        frame.returnBoolean(result);
    }

    //native Class<? super T> getSuperclass();
    void getSuperclass(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto superClass = mirrorClass->superClass;
        if (superClass == nullptr) {
            frame.returnRef(nullptr);
        } else {
            frame.returnRef(superClass->getMirrorOop());
        }
    }

    //native Class<?> getComponentType();
    void getComponentType(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass->type != ClassTypeEnum::TypeArrayClass && mirrorClass->type != ClassTypeEnum::ObjArrayClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto arrayClass = CAST_ARRAY_CLASS(mirrorClass);
        const auto componentName = arrayClass->getComponentClassName();
        const auto componentMirrorOop = frame.getCurrentClassLoader()->getClass(componentName)->getMirrorOop();
        frame.returnRef(componentMirrorOop);
    }

    //native Object[] getEnclosingMethod0();
    void getEnclosingMethod0(Frame &frame) {
        //Class Instance
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto enclosingMethodAttr = mirrorInstanceClass->getEnclosingMethodAttr();
        if (enclosingMethodAttr == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, enclosingMethodAttr->classIndex);
        const auto objArrayOop = 
            frame.vm.oopManager->newObjArrayOop(
                frame.classLoader.getObjectArrayClass(JAVA_LANG_OBJECT_NAME), 
                3
            );
        objArrayOop->data[0] = frame.vm.bootstrapClassLoader->getClass(className)->getMirrorOop();

        const auto [methodName, methodDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, enclosingMethodAttr->methodIndex);
        objArrayOop->data[1] = frame.vm.stringPool->getInternString(methodName);
        objArrayOop->data[2] = frame.vm.stringPool->getInternString(methodDescriptor);

        frame.returnRef(objArrayOop);
    }

    //native Class<?> getDeclaringClass0();
    void getDeclaringClass0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto innerClassesAttr = mirrorInstanceClass->getInnerClassesAttr();
        if (innerClassesAttr == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        for (const auto &item : innerClassesAttr->classes) {
            if (item->innerClassInfoIndex != 0) {
                const auto innerClassName = getConstantStringFromPoolByIndexInfo(constantPool, item->innerClassInfoIndex);
                if (innerClassName == mirrorInstanceClass->name) {
                    if (item->outerClassInfoIndex == 0) {
                        frame.returnRef(nullptr);
                        return;
                    } else {
                        const auto outerClassName = getConstantStringFromPoolByIndexInfo(constantPool, item->outerClassInfoIndex);
                        frame.returnRef(frame.vm.bootstrapClassLoader->getClass(outerClassName)->getMirrorOop());
                        return;
                    }
                } 
            }
        }
        frame.returnRef(nullptr);
    }

    //Field[] getDeclaredFields0(boolean publicOnly)
    void getDeclaredFields0(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto &stringPool = frame.vm.stringPool;
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }

        const auto retTypeClass = classLoader.getInstanceClass("java/lang/reflect/Field");
        const auto retArrayTypeClass = classLoader.getObjectArrayClass("java/lang/reflect/Field");

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &fields = instanceMirrorClass->fields;
        std::vector<InstanceOop *> fieldInstances;
        for (const auto &field : fields) {
            if (publicOnly && !field->isPublic()) {
                continue;
            }

            auto mirrorFieldInstance = oopManager->newInstance(retTypeClass);
            mirrorFieldInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(instanceMirrorClass->getMirrorOop()));
            mirrorFieldInstance->setFieldValue("slot", "I", Slot(field->slotId));
            mirrorFieldInstance->setFieldValue("name", "Ljava/lang/String;", Slot(stringPool->getInternString(field->name)));
            mirrorFieldInstance->setFieldValue("type", "Ljava/lang/Class;", Slot(field->getTypeClass()->getMirrorOop()));
            mirrorFieldInstance->setFieldValue("modifiers", "I", Slot(field->getModifier()));
            mirrorFieldInstance->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.vm.stringPool->getInternString(field->signature)));
            if (field->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        field->runtimeVisibleAnnotationLength, 
                        field->runtimeVisibleAnnotation.get()
                    );
                mirrorFieldInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }
            fieldInstances.emplace_back(mirrorFieldInstance);
        }

        const auto retArrayOop = oopManager->newObjArrayOop(retArrayTypeClass, fieldInstances.size());
        for (size_t i = 0; i < fieldInstances.size(); ++i) {
            retArrayOop->data[i] = fieldInstances.at(i);
        }

        frame.returnRef(retArrayOop);
    }

    void getDeclaredCommons0(Frame &frame, bool isConstructor) {
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &stringPool = frame.vm.stringPool;
        const auto classArrayClass = classLoader.getObjectArrayClass(JAVA_LANG_CLASS_NAME);

        const auto retTypeClassName = 
            isConstructor ? 
                "java/lang/reflect/Constructor" : 
                "java/lang/reflect/Method";

        const auto retTypeClass = classLoader.getInstanceClass(retTypeClassName);
        const auto retTypeArrayClass = classLoader.getObjectArrayClass(retTypeClassName);
        
        const auto &methods = instanceMirrorClass->methods;
        std::vector<InstanceOop *> methodInstances;
        for (const auto &method : methods) {
            if (publicOnly && !method->isPublic()) {
                continue;
            }
            if (isConstructor) {
                if (method->name != "<init>") {
                    continue;
                }
            } else {
                if (method->name == "<init>" || method->name == "<clinit>") {
                    continue;
                }
            }

            const auto paramClasses = method->getParamClasses();
            const auto paramClassesArrayOop = oopManager->newObjArrayOop(classArrayClass, paramClasses.size());
            for (size_t i = 0; i < paramClasses.size(); ++i) {
                paramClassesArrayOop->data[i] = paramClasses.at(i)->getMirrorOop();
            }

            const auto exceptionArrayOop = oopManager->newObjArrayOop(classArrayClass, method->exceptionsIndex.size());
            for (size_t i = 0; i <  method->exceptionsIndex.size(); ++i) {
                const auto exceptionIdx = method->exceptionsIndex.at(i);
                const auto exceptionClassName = 
                    getConstantStringFromPoolByIndexInfo(instanceMirrorClass->constantPool, exceptionIdx);
                exceptionArrayOop->data[i] = frame.getCurrentClassLoader()->getClass(exceptionClassName)->getMirrorOop();
            }

            auto mirrorMethodInstance = oopManager->newInstance(retTypeClass);
            mirrorMethodInstance->setFieldValue("override", "Z", Slot(CAST_I4(0)));
            mirrorMethodInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(instanceMirrorClass->getMirrorOop()));
            mirrorMethodInstance->setFieldValue("slot", "I", Slot(method->index));
            mirrorMethodInstance->setFieldValue("parameterTypes", "[Ljava/lang/Class;", Slot(paramClassesArrayOop));
            mirrorMethodInstance->setFieldValue("exceptionTypes", "[Ljava/lang/Class;", Slot(exceptionArrayOop));
            mirrorMethodInstance->setFieldValue("modifiers", "I", Slot(method->getModifier()));
            mirrorMethodInstance->setFieldValue("signature", "Ljava/lang/String;", Slot(stringPool->getInternString(method->signature)));

            if (!isConstructor) {
                mirrorMethodInstance->setFieldValue("name", "Ljava/lang/String;", Slot(stringPool->getInternString(method->name)));
                mirrorMethodInstance->setFieldValue("returnType", "Ljava/lang/Class;", Slot(classLoader.getClass(method->returnType)->getMirrorOop()));
            }
            
            if (method->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        method->runtimeVisibleAnnotationLength, 
                        method->runtimeVisibleAnnotation.get()
                    );
                mirrorMethodInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }

            if (method->runtimeVisibleParameterAnnotation != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        method->runtimeVisibleParameterAnnotationLength, 
                        method->runtimeVisibleParameterAnnotation.get()
                    );
                mirrorMethodInstance->setFieldValue("parameterAnnotations", "[B", Slot(byteArrayOop));
            }

            if (!isConstructor && method->annotationDefault != nullptr) {
                const auto byteArrayOop = 
                    oopManager->newByteArrayOop(
                        method->annotationDefaultLength, 
                        method->annotationDefault.get()
                    );
                mirrorMethodInstance->setFieldValue("annotationDefault", "[B", Slot(byteArrayOop));
            }
            methodInstances.emplace_back(mirrorMethodInstance);
        }

        const auto retArrayOop = oopManager->newObjArrayOop(retTypeArrayClass, methodInstances.size());
        for (size_t i = 0; i < methodInstances.size(); ++i) {
            retArrayOop->data[i] = methodInstances.at(i);
        }

        frame.returnRef(retArrayOop);
    }

    void getDeclaredConstructors0(Frame &frame) {
        getDeclaredCommons0(frame, true);
    }

    void getDeclaredMethods0(Frame &frame) {
        getDeclaredCommons0(frame, false);
    }

    void getInterfaces0(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto classArrayClass = classLoader.getObjectArrayClass(JAVA_LANG_CLASS_NAME);

        const auto &interfaces = instanceMirrorClass->interfaces;
        const auto retArrayOop = oopManager->newObjArrayOop(classArrayClass, interfaces.size());
         for (size_t i = 0; i < interfaces.size(); ++i) {
            retArrayOop->data[i] = interfaces.at(i)->getMirrorOop();
        }

        frame.returnRef(retArrayOop);
    }

    //native Object[] getSigners();
    void getSigners(Frame &frame) {
        frame.returnRef(nullptr);
    }

    //native void setSigners(Object[] signers);
    void setSigners(Frame &frame) {
    }

    //native java.security.ProtectionDomain getProtectionDomain0();
    void getProtectionDomain0(Frame &frame) {
        frame.returnRef(nullptr);
    }

    //native String getGenericSignature0();
    void getGenericSignature0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto &stringPool = frame.vm.stringPool;
        const auto signature = (CAST_INSTANCE_CLASS(mirrorClass))->signature;
        if (signature != EMPTY_STRING) {
            frame.returnRef(stringPool->getInternString(signature));
            return;
        }

        frame.returnRef(nullptr);
    }

    //native byte[] getRawAnnotations();
    void getRawAnnotations(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        if (instanceMirrorClass->runtimeVisibleAnnotation != nullptr) {
            const auto byteArrayOop = 
                oopManager->newByteArrayOop(
                    instanceMirrorClass->runtimeVisibleAnnotationLength, 
                    instanceMirrorClass->runtimeVisibleAnnotation.get()
                );

            frame.returnRef(byteArrayOop);
            return;
        }
        frame.returnRef(nullptr);
    }

    //native byte[] getRawTypeAnnotations(); 
    void getRawTypeAnnotations(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        if (instanceMirrorClass->runtimeVisibleTypeAnnotation != nullptr) {
            const auto byteArrayOop = 
                oopManager->newByteArrayOop(
                    instanceMirrorClass->runtimeVisibleTypeAnnotationLength, 
                    instanceMirrorClass->runtimeVisibleTypeAnnotation.get()
                );

            frame.returnRef(byteArrayOop);
            return;
        }
        frame.returnRef(nullptr);
    }

    //native byte[] getTypeAnnotationBytes0();
    void fieldGetTypeAnnotationBytes0(Frame &frame) {
        const auto &oopManager = frame.vm.oopManager;
        const auto self = frame.getThisInstance();
        const auto slotId = self->getFieldValue("slot", "I").i4Val;
        const auto fieldClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(self->getFieldValue("clazz", "Ljava/lang/Class;").refVal)->mirrorClass);
        const auto fieldPtr = fieldClass->fields[slotId].get();

        if (fieldPtr->runtimeVisibleTypeAnnotation != nullptr) {
            const auto byteArrayOop = 
                oopManager->newByteArrayOop(
                    fieldPtr->runtimeVisibleTypeAnnotationLength, 
                    fieldPtr->runtimeVisibleTypeAnnotation.get()
                );

            frame.returnRef(byteArrayOop);
            return;
        }

        frame.returnRef(nullptr);
    }

    //native ConstantPool getConstantPool();
    void getConstantPool(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThisInstance());
        const auto mirrorClass = instance->klass;
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        auto constantPoolPtr = instance->constantPoolOop.get();
        if (constantPoolPtr == nullptr) {
            auto &classLoader = *frame.getCurrentClassLoader();
            const auto constantPoolClass = classLoader.getInstanceClass("sun/reflect/ConstantPool");
            instance->constantPoolOop = std::make_unique<InstanceOop>(constantPoolClass);
            instance->constantPoolOop->setFieldValue("constantPoolOop", "Ljava/lang/Object;", Slot(instance));
            constantPoolPtr = instance->constantPoolOop.get();
        }
        frame.returnRef(instance->constantPoolOop.get());
    }

    //private native Class<?>[] getDeclaredClasses0();
    void getDeclaredClasses0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::InstanceClass) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto innerClassesAttr = mirrorInstanceClass->getInnerClassesAttr();
        const auto classArrayClass = classLoader.getObjectArrayClass(JAVA_LANG_CLASS_NAME);

        if (innerClassesAttr == nullptr || innerClassesAttr->classes.empty()) {
            const auto emptyOop = oopManager->newObjArrayOop(classArrayClass, 0);
            frame.returnRef(emptyOop);
            return;
        }
        
        const auto retArrayOop = oopManager->newObjArrayOop(classArrayClass, innerClassesAttr->classes.size());
        for (size_t i = 0; i < innerClassesAttr->classes.size(); ++i) {
            const auto innerClassName = 
                getConstantStringFromPoolByIndexInfo(constantPool, innerClassesAttr->classes.at(i)->innerClassInfoIndex);
            retArrayOop->data[i] = classLoader.getClass(innerClassName)->getMirrorOop();
        }
            
        frame.returnRef(retArrayOop);
    }

    
    //native Class<?> getClassAt0         (Object constantPoolOop, int index);
    void getClassAt0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;

        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        auto &classLoader = *frame.getCurrentClassLoader();

        const auto val = classLoader.getClass(className)->getMirrorOop();
        frame.returnRef(val);
    }

    //native int      getIntAt0           (Object constantPoolOop, int index);
    void getIntAt0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_INTEGER_INFO(ptr)->value;
        frame.returnI4(val);
    }

    //native long     getLongAt0          (Object constantPoolOop, int index);
    void getLongAt0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_LONG_INFO(ptr)->value;
        frame.returnI8(val);
    }

    //native float    getFloatAt0         (Object constantPoolOop, int index);
    void getFloatAt0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_FLOAT_INFO(ptr)->value;
        frame.returnF4(val);
    }

    //native double   getDoubleAt0        (Object constantPoolOop, int index);
    void getDoubleAt0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_DOUBLE_INFO(ptr)->value;
        frame.returnF8(val);
    }

    //native String   getUTF8At0          (Object constantPoolOop, int index);
    void getUTF8At0(Frame &frame) {
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->mirrorClass);
        const auto &constantPool = mirrorClass->constantPool;

        const auto val = getConstantStringFromPool(constantPool, index);
        const auto &stringPool = frame.vm.stringPool;
        frame.returnRef(stringPool->getInternString(val));
    }

    void invoke0(Frame &frame) {
        auto &classLoader = *frame.getCurrentClassLoader();
        const auto &oopManager = frame.vm.oopManager;
        const auto method = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto obj = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto args = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(2));
        if (method == nullptr) {
            panic("invoke0 error: method nullptr");
        }

        const auto methodClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(method->getFieldValue("clazz", "Ljava/lang/Class;").refVal)->mirrorClass);
        const auto methodName = StringPool::getJavaString(CAST_INSTANCE_OOP(method->getFieldValue("name", "Ljava/lang/String;").refVal));
        const auto slotId = method->getFieldValue("slot", "I").i4Val;
        const auto methodPtr = methodClass->methods[slotId].get();

        std::vector<Slot> params;
        if (!methodPtr->isStatic()) {
            if (obj == nullptr) {
                panic("invoke0 error: obj nullptr");
            }
            params.emplace_back(Slot(obj));
        }
        for (size_t i = 0; i < args->dataLength; ++i) {
            const auto paramType = methodPtr->paramType[i];
            const auto val = CAST_INSTANCE_OOP(args->data[i]);
            const auto paramClass = classLoader.getClass(paramType);
            if (paramClass->type == ClassTypeEnum::PrimitiveClass) {
                const auto primitiveClass = CAST_PRIMITIVE_CLASS(paramClass);
                const auto slotVal = primitiveClass->getValueFromBoxingOop(val);
                params.emplace_back(slotVal);
                if (primitiveClass->isWideType()) {
                    params.emplace_back(Slot(0));
                }
            } else {
                params.emplace_back(Slot(val));
            }
        }
        const auto [result, slotType] = frame.runMethodManual(*methodPtr, params);
        const auto returnClass = classLoader.getClass(methodPtr->returnType);

        ref oopResult = nullptr;
        if (slotType != SlotTypeEnum::NONE) {
            if (returnClass->type == ClassTypeEnum::PrimitiveClass) {
                const auto returnPrimitiveClass = CAST_PRIMITIVE_CLASS(returnClass);
                oopResult = returnPrimitiveClass->getBoxingOopFromValue(result, *oopManager);
            } else {
                oopResult = result.refVal;
            }
        }

        frame.returnRef(oopResult);
    }

    void getCallerClass(Frame &frame) {
        const auto callerFrame = frame.previous->previous;
        frame.returnRef(callerFrame->klass.getMirrorOop());
    }
    
    void getClassAccessFlags(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThis());
        const auto mirrorClass = instance->mirrorClass;
        frame.returnI4(CAST_I4(mirrorClass->accessFlags));
    }

}


#endif