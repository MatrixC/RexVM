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
#include "../../string_pool.hpp"
#include "../../exception_helper.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/class_utils.hpp"

namespace RexVM::Native::Core {

    Class *getMirrorClass(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThisInstance());
        return instance->getMirrorClass();
    }

    InstanceClass *getMirrorInstanceClass(Frame &frame) {
        return CAST_INSTANCE_CLASS(getMirrorClass(frame));
    }

    //native String getName0();
    void getName0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto className = mirrorClass->name;
        const auto javaClassName = getJavaClassName(className);
        auto strOop = frame.mem.getInternString(javaClassName);
        frame.returnRef(strOop);
    }

    //static Class<?> forName0(String className, boolean initialize, ClassLoader loader, Class<?> caller)
    void forName0(Frame &frame) {
        auto classNameOop = frame.getLocalRef(0);
        const auto className = StringPool::getJavaString(CAST_INSTANCE_OOP(classNameOop));
        const auto initialize = frame.getLocalI4(1);

        const auto jvmClassName = getJVMClassName(className);
        const auto klass = frame.mem.getClass(jvmClassName);
        if (klass == nullptr) {
            throwClassNotFoundException(frame, className);
            return;
        }
        if (initialize && klass->getType() == ClassTypeEnum::INSTANCE_CLASS) {
            CAST_INSTANCE_CLASS(klass)->clinit(frame);
        }
        frame.returnRef(klass->getMirror(&frame));
    }

    //static native Class<?> getPrimitiveClass(String name);
    void getPrimitiveClass(Frame &frame) {
        auto classNameOop = frame.getThisInstance();
        const auto className = StringPool::getJavaString(classNameOop);
        const auto klass = frame.mem.getClass(className);
        frame.returnRef(klass->getMirror(&frame));
    }

    //static native boolean desiredAssertionStatus0(Class<?> clazz);
    void desiredAssertionStatus0(Frame &frame) {
        frame.returnBoolean(false);
    }

    //native boolean isPrimitive();
    void isPrimitive(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->type == ClassTypeEnum::PRIMITIVE_CLASS);
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
        const auto mirrorThatClass = thatClass->getMirrorClass();
        frame.returnBoolean(mirrorClass->isAssignableFrom(mirrorThatClass));
    }

    //native int getModifiers();
    void getModifiers(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto removeSuper = ~CAST_I4(AccessFlagEnum::ACC_SUPER);
        frame.returnI4(CAST_I4(mirrorClass->getAccessFlags()) & removeSuper);
    }

    //native boolean isInstance(Object obj);
    void isInstance(Frame &frame) {
        const auto thisClass = getMirrorClass(frame);
        const auto objOop = frame.getLocalRef(1);
        if (objOop == nullptr) {
            frame.returnBoolean(false);
            return;
        }
        const auto objClass = objOop->getClass();
        const auto result = objClass->isInstanceOf(thisClass);
        frame.returnBoolean(result);
    }

    //native Class<? super T> getSuperclass();
    void getSuperclass(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        const auto superClass = mirrorClass->getSuperClass();
        if (superClass == nullptr) {
            frame.returnRef(nullptr);
        } else {
            frame.returnRef(superClass->getMirror(&frame));
        }
    }

    //native Class<?> getComponentType();
    void getComponentType(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass->type != ClassTypeEnum::TYPE_ARRAY_CLASS && mirrorClass->type != ClassTypeEnum::OBJ_ARRAY_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto arrayClass = CAST_ARRAY_CLASS(mirrorClass);
        const auto componentName = arrayClass->getComponentClassName();
        const auto componentMirrorOop = frame.mem.getClass(componentName)->getMirror(&frame);
        frame.returnRef(componentMirrorOop);
    }

    //native Object[] getEnclosingMethod0();
    void getEnclosingMethod0(Frame &frame) {
        //Class Instance
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
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
        const auto targetClass = frame.mem.getClass(className);
        if (targetClass == nullptr || targetClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto targetInstanceClass = CAST_INSTANCE_CLASS(targetClass);

        const auto objArrayOop = frame.mem.newObjectObjArrayOop(3);
        objArrayOop->data[0] = targetInstanceClass->getMirror(&frame);

        if (enclosingMethodAttr->methodIndex != 0) {
            const auto [methodName, methodDescriptor] = getConstantStringFromPoolByNameAndType(constantPool, enclosingMethodAttr->methodIndex);
            objArrayOop->data[1] = frame.mem.getInternString(methodName);
            objArrayOop->data[2] = frame.mem.getInternString(methodDescriptor);
        }

        frame.returnRef(objArrayOop);
    }

    //native Class<?> getDeclaringClass0();
    void getDeclaringClass0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
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
                        frame.returnRef(frame.mem.getClass(outerClassName)->getMirror(&frame));
                        return;
                    }
                } 
            }
        }
        frame.returnRef(nullptr);
    }

    //Field[] getDeclaredFields0(boolean publicOnly)
    void getDeclaredFields0(Frame &frame) {
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }

        const auto retTypeClass = frame.mem.getInstanceClass("java/lang/reflect/Field");
        const auto retArrayTypeClass = frame.mem.getObjectArrayClass("java/lang/reflect/Field");

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &fields = instanceMirrorClass->fields;
        std::vector<InstanceOop *> fieldInstances;
        for (const auto &field : fields) {
            if (publicOnly && !field->isPublic()) {
                continue;
            }

            auto mirrorFieldInstance = frame.mem.newInstance(retTypeClass);
            mirrorFieldInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(instanceMirrorClass->getMirror(&frame)));
            mirrorFieldInstance->setFieldValue("slot", "I", Slot(field->slotId));
            mirrorFieldInstance->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->name)));
            mirrorFieldInstance->setFieldValue("type", "Ljava/lang/Class;", Slot(field->getTypeClass()->getMirror(&frame)));
            mirrorFieldInstance->setFieldValue("modifiers", "I", Slot(field->getModifier()));
            mirrorFieldInstance->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.mem.getInternString(field->signature)));
            if (field->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = 
                    frame.mem.newByteArrayOop(
                        field->runtimeVisibleAnnotationLength, 
                        field->runtimeVisibleAnnotation.get()
                    );
                mirrorFieldInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }
            fieldInstances.emplace_back(mirrorFieldInstance);
        }

        const auto retArrayOop = frame.mem.newObjArrayOop(retArrayTypeClass, fieldInstances.size());
        for (size_t i = 0; i < fieldInstances.size(); ++i) {
            retArrayOop->data[i] = fieldInstances.at(i);
        }

        frame.returnRef(retArrayOop);
    }

    void getDeclaredCommons0(Frame &frame, bool isConstructor) {
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        const auto retTypeClassName = 
            isConstructor ? 
                "java/lang/reflect/Constructor" : 
                "java/lang/reflect/Method";

        const auto retTypeClass = frame.mem.getInstanceClass(retTypeClassName);
        const auto retTypeArrayClass = frame.mem.getObjectArrayClass(retTypeClassName);
        
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
            const auto paramClassesArrayOop = frame.mem.newClassObjArrayOop(paramClasses.size());
            for (size_t i = 0; i < paramClasses.size(); ++i) {
                paramClassesArrayOop->data[i] = paramClasses.at(i)->getMirror(&frame);
            }

            const auto exceptionArrayOop = frame.mem.newClassObjArrayOop(method->exceptionsIndex.size());
            for (size_t i = 0; i <  method->exceptionsIndex.size(); ++i) {
                const auto exceptionIdx = method->exceptionsIndex.at(i);
                const auto exceptionClassName = 
                    getConstantStringFromPoolByIndexInfo(instanceMirrorClass->constantPool, exceptionIdx);
                exceptionArrayOop->data[i] = frame.mem.getClass(exceptionClassName)->getMirror(&frame);
            }

            auto mirrorMethodInstance = frame.mem.newInstance(retTypeClass);
            mirrorMethodInstance->setFieldValue("override", "Z", Slot(CAST_I4(0)));
            mirrorMethodInstance->setFieldValue("clazz", "Ljava/lang/Class;", Slot(instanceMirrorClass->getMirror(&frame)));
            mirrorMethodInstance->setFieldValue("slot", "I", Slot(method->index));
            mirrorMethodInstance->setFieldValue("parameterTypes", "[Ljava/lang/Class;", Slot(paramClassesArrayOop));
            mirrorMethodInstance->setFieldValue("exceptionTypes", "[Ljava/lang/Class;", Slot(exceptionArrayOop));
            mirrorMethodInstance->setFieldValue("modifiers", "I", Slot(method->getModifier()));
            if (!method->signature.empty()) {
                mirrorMethodInstance->setFieldValue("signature", "Ljava/lang/String;", Slot(frame.mem.getInternString(method->signature)));
            }

            if (!isConstructor) {
                mirrorMethodInstance->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(method->name)));
                mirrorMethodInstance->setFieldValue("returnType", "Ljava/lang/Class;", Slot(frame.mem.getClass(method->returnType)->getMirror(&frame)));
            }
            
            if (method->runtimeVisibleAnnotation != nullptr) {
                const auto byteArrayOop = 
                    frame.mem.newByteArrayOop(
                        method->runtimeVisibleAnnotationLength, 
                        method->runtimeVisibleAnnotation.get()
                    );
                mirrorMethodInstance->setFieldValue("annotations", "[B", Slot(byteArrayOop));
            }

            if (method->runtimeVisibleParameterAnnotation != nullptr) {
                const auto byteArrayOop = 
                    frame.mem.newByteArrayOop(
                        method->runtimeVisibleParameterAnnotationLength, 
                        method->runtimeVisibleParameterAnnotation.get()
                    );
                mirrorMethodInstance->setFieldValue("parameterAnnotations", "[B", Slot(byteArrayOop));
            }

            if (!isConstructor && method->annotationDefault != nullptr) {
                const auto byteArrayOop = 
                    frame.mem.newByteArrayOop(
                        method->annotationDefaultLength, 
                        method->annotationDefault.get()
                    );
                mirrorMethodInstance->setFieldValue("annotationDefault", "[B", Slot(byteArrayOop));
            }
            methodInstances.emplace_back(mirrorMethodInstance);
        }

        const auto retArrayOop = frame.mem.newObjArrayOop(retTypeArrayClass, methodInstances.size());
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
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        const auto interfaceSize = instanceMirrorClass->getInterfaceSize();
        const auto retArrayOop = frame.mem.newClassObjArrayOop(interfaceSize);
        FOR_FROM_ZERO(interfaceSize) {
            retArrayOop->data[i] = instanceMirrorClass->getInterfaceByIndex(i)->getMirror(&frame);
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
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto signature = (CAST_INSTANCE_CLASS(mirrorClass))->signature;
        if (!signature.empty()) {
            frame.returnRef(frame.mem.getInternString(signature));
            return;
        }

        frame.returnRef(nullptr);
    }

    //native byte[] getRawAnnotations();
    void getRawAnnotations(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        if (instanceMirrorClass->runtimeVisibleAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
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
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        if (instanceMirrorClass->runtimeVisibleTypeAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
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
        const auto self = frame.getThisInstance();
        const auto slotId = self->getFieldValue("slot", "I").i4Val;
        const auto fieldClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(self->getFieldValue("clazz", "Ljava/lang/Class;").refVal)->getMirrorClass());
        const auto fieldPtr = fieldClass->fields[slotId].get();

        if (fieldPtr->runtimeVisibleTypeAnnotation != nullptr) {
            const auto byteArrayOop = 
                frame.mem.newByteArrayOop(
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
        const auto instanceClass = instance->getClass();
        if (instanceClass == nullptr || instanceClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorClass = CAST_INSTANCE_CLASS(instance->getMirrorClass());
        //auto constantPoolPtr = instance->constantPoolOop.get();
        auto constantPoolPtr = mirrorClass->constantPoolOop.get();
    
        if (constantPoolPtr == nullptr) {
            //TODO Thread unsafe
            const auto constantPoolClass = frame.mem.getInstanceClass("sun/reflect/ConstantPool");
            mirrorClass->constantPoolOop = std::make_unique<InstanceOop>(constantPoolClass);
            mirrorClass->constantPoolOop->setFieldValue("constantPoolOop", "Ljava/lang/Object;", Slot(instance));
            constantPoolPtr = mirrorClass->constantPoolOop.get();
        }
        frame.returnRef(constantPoolPtr);
    }

    //private native Class<?>[] getDeclaredClasses0();
    void getDeclaredClasses0(Frame &frame) {
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorInstanceClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &constantPool = mirrorInstanceClass->constantPool;
        const auto innerClassesAttr = mirrorInstanceClass->getInnerClassesAttr();

        if (innerClassesAttr == nullptr || innerClassesAttr->classes.empty()) {
            const auto emptyOop = frame.mem.newClassObjArrayOop( 0);
            frame.returnRef(emptyOop);
            return;
        }
        
        const auto retArrayOop = frame.mem.newClassObjArrayOop(innerClassesAttr->classes.size());
        for (size_t i = 0; i < innerClassesAttr->classes.size(); ++i) {
            const auto innerClassName = 
                getConstantStringFromPoolByIndexInfo(constantPool, innerClassesAttr->classes.at(i)->innerClassInfoIndex);
            retArrayOop->data[i] = frame.mem.getClass(innerClassName)->getMirror(&frame);
        }
            
        frame.returnRef(retArrayOop);
    }

    //private native Parameter[] getParameters0();
    void getParameters0(Frame &frame) {
        const auto method = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(method);
        const auto methodClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(method->getFieldValue("clazz", "Ljava/lang/Class;").refVal)->getMirrorClass());
        const auto slotId = method->getFieldValue("slot", "I").i4Val;
        const auto methodPtr = methodClass->methods[slotId].get();
        const auto parameterClass = frame.mem.getInstanceClass("java/lang/reflect/Parameter");
        const auto result = frame.mem.newObjArrayOop(frame.mem.getObjectArrayClass("java/lang/reflect/Parameter"), methodPtr->paramType.size());

        for (size_t i = 0; i < methodPtr->paramType.size(); ++i) {
            const auto parameter = frame.mem.newInstance(parameterClass);
            const auto name = cformat("arg{}", i);
            parameter->setFieldValue("name", "Ljava/lang/String;", Slot(frame.mem.getInternString(name)));
            parameter->setFieldValue("modifiers", "I", Slot(CAST_I4(0)));
            parameter->setFieldValue("executable", "Ljava/lang/reflect/Executable;", Slot(method));
            parameter->setFieldValue("index", "I", Slot(CAST_I4(i)));
            result->data[i] = parameter;
        }
        
        frame.returnRef(result);
    }

    
    //native Class<?> getClassAt0         (Object constantPoolOop, int index);
    void getClassAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;

        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);

        const auto val = frame.mem.getClass(className)->getMirror(&frame);
        frame.returnRef(val);
    }

    //native int      getIntAt0           (Object constantPoolOop, int index);
    void getIntAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_INTEGER_INFO(ptr)->value;
        frame.returnI4(val);
    }

    //native long     getLongAt0          (Object constantPoolOop, int index);
    void getLongAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_LONG_INFO(ptr)->value;
        frame.returnI8(val);
    }

    //native float    getFloatAt0         (Object constantPoolOop, int index);
    void getFloatAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_FLOAT_INFO(ptr)->value;
        frame.returnF4(val);
    }

    //native double   getDoubleAt0        (Object constantPoolOop, int index);
    void getDoubleAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool.at(index).get();

        const auto val = CAST_CONSTANT_DOUBLE_INFO(ptr)->value;
        frame.returnF8(val);
    }

    //native String   getUTF8At0          (Object constantPoolOop, int index);
    void getUTF8At0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto mirrorOop = CAST_MIRROR_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(mirrorOop);
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(mirrorOop->getMirrorClass());
        const auto &constantPool = mirrorClass->constantPool;

        const auto val = getConstantStringFromPool(constantPool, index);
        frame.returnRef(frame.mem.getInternString(val));
    }

    void invoke0(Frame &frame) {
        const auto method = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        ASSERT_IF_NULL_THROW_NPE(method);
        const auto obj = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto args = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(2));

        const auto methodClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(method->getFieldValue("clazz", "Ljava/lang/Class;").refVal)->getMirrorClass());
        //const auto methodName = StringPool::getJavaString(CAST_INSTANCE_OOP(method->getFieldValue("name", "Ljava/lang/String;").refVal));
        const auto slotId = method->getFieldValue("slot", "I").i4Val;
        const auto methodPtr = methodClass->methods[slotId].get();

        std::vector<Slot> params;
        if (!methodPtr->isStatic()) {
            if (obj == nullptr) {
                panic("invoke0 error: obj nullptr");
            }
            params.emplace_back(Slot(obj));
        }
        for (size_t i = 0; i < args->getDataLength(); ++i) {
            const auto paramType = methodPtr->paramType[i];
            const auto val = CAST_INSTANCE_OOP(args->data[i]);
            const auto paramClass = frame.mem.getClass(paramType);
            if (paramClass->type == ClassTypeEnum::PRIMITIVE_CLASS) {
                const auto primitiveClass = CAST_PRIMITIVE_CLASS(paramClass);
                const auto slotVal = primitiveClass->getValueFromBoxingOop(val);
                params.emplace_back(slotVal);
                if (primitiveClass->isWideType()) {
                    params.emplace_back(ZERO_SLOT);
                }
            } else {
                params.emplace_back(val);
            }
        }
        const auto [result, slotType] = frame.runMethodManual(*methodPtr, params);
        const auto returnClass = frame.mem.getClass(methodPtr->returnType);

        ref oopResult = nullptr;
        if (slotType != SlotTypeEnum::NONE) {
            if (returnClass->type == ClassTypeEnum::PRIMITIVE_CLASS) {
                const auto returnPrimitiveClass = CAST_PRIMITIVE_CLASS(returnClass);
                oopResult = returnPrimitiveClass->getBoxingOopFromValue(result, frame);
            } else {
                oopResult = result.refVal;
            }
        }

        frame.returnRef(oopResult);
    }

    void getCallerClass(Frame &frame) {
        const auto callerFrame = frame.previous->previous;
        frame.returnRef(callerFrame->klass.getMirror(&frame));
    }
    
    void getClassAccessFlags(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThis());
        const auto mirrorClass = instance->getMirrorClass();
        frame.returnI4(CAST_I4(mirrorClass->getAccessFlags()));
    }

}


#endif