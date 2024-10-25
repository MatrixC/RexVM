#ifndef NATIVE_CORE_JAVA_LANG_CLASS_HPP
#define NATIVE_CORE_JAVA_LANG_CLASS_HPP
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../class_member.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../string_pool.hpp"
#include "../../exception_helper.hpp"
#include "../../attribute_info.hpp"
#include "../../utils/string_utils.hpp"
#include "../../utils/class_utils.hpp"

namespace RexVM::Native::Core {

    Class *getMirrorClass(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThisInstance());
        return instance->getMirrorClass();
    }

    //native String getName0();
    void getName0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        const auto className = mirrorClass->getClassName();
        const auto javaClassName = getJavaClassName(className);
        auto strOop = frame.mem.getInternString(javaClassName);
        frame.returnRef(strOop);
    }

    //static Class<?> forName0(String className, boolean initialize, ClassLoader loader, Class<?> caller)
    void forName0(Frame &frame) {
        const auto classNameOop = frame.getLocalRef(0);
        ASSERT_IF_NULL_THROW_NPE(classNameOop)
        const auto className = VMStringHelper::getJavaString(CAST_INSTANCE_OOP(classNameOop));
        const auto initialize = frame.getLocalI4(1);

        const auto jvmClassName = getJVMClassName(className);
        const auto klass = frame.mem.getClass(jvmClassName);
        if (klass == nullptr) {
            throwClassNotFoundException(frame, className);
            return;
        }
        if (initialize && klass->getType() == ClassTypeEnum::INSTANCE_CLASS) {
            CAST_INSTANCE_CLASS(klass)->clinit(frame);
            if (frame.markThrow) {
                return;
            }
        }
        frame.returnRef(klass->getMirror(&frame));
    }

    //static native Class<?> getPrimitiveClass(String name);
    void getPrimitiveClass(Frame &frame) {
        const auto classNameOop = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(classNameOop)
        const auto className = VMStringHelper::getJavaString(classNameOop);
        const auto klass = frame.mem.getClass(className);
        frame.returnRef(klass->getMirror(&frame));
    }

    //static native boolean desiredAssertionStatus0(Class<?> clazz);
    void desiredAssertionStatus0(Frame &frame) {
        frame.returnBoolean(false);
    }

    //native boolean isPrimitive();
    void isPrimitive(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->type == ClassTypeEnum::PRIMITIVE_CLASS);
    }

    //native boolean isInterface();
    void isInterface(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->isInterface());
    }

    //native boolean isArray();
    void isArray(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        frame.returnBoolean(mirrorClass->isArray());
    }

    //native boolean isAssignableFrom(Class<?> cls);
    void isAssignableFrom(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        const auto thatClass = CAST_MIRROR_OOP(frame.getLocalRef(1));
        const auto mirrorThatClass = thatClass->getMirrorClass();
        frame.returnBoolean(mirrorClass->isAssignableFrom(mirrorThatClass));
    }

    //native int getModifiers();
    void getModifiers(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        const auto removeSuper = ~CAST_I4(AccessFlagEnum::ACC_SUPER);
        frame.returnI4(CAST_I4(mirrorClass->getAccessFlags()) & removeSuper);
    }

    //native boolean isInstance(Object obj);
    void isInstance(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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


    //Field[] getDeclaredFields0(boolean publicOnly)
    void getDeclaredFields0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }

        const auto fieldClass = frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_FIELD);
        const auto retArrayTypeClass = frame.mem.getObjectArrayClass(*fieldClass);

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        const auto &fields = instanceMirrorClass->fields;
        std::vector<InstanceOop *> fieldInstances;
        fieldInstances.reserve(fields.size());
        for (const auto &field : fields) {
            if (publicOnly && !field->isPublic()) {
                continue;
            }

            const auto mirrorFieldInstance = field->getMirror(&frame);
            fieldInstances.emplace_back(mirrorFieldInstance);
        }

        const auto retArrayOop = frame.mem.newObjArrayOop(retArrayTypeClass, fieldInstances.size());
        for (size_t i = 0; i < fieldInstances.size(); ++i) {
            retArrayOop->data[i] = fieldInstances[i];
        }

        frame.returnRef(retArrayOop);
    }

    //native Class<?> getDeclaringClass0();
    void getDeclaringClass0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
                if (innerClassName == mirrorInstanceClass->getClassName()) {
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

    void getDeclaredCommons0(Frame &frame, bool isConstructor) {
        const auto publicOnly = frame.getLocalBoolean(1);
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }

        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        const auto retTypeClass = 
            isConstructor ?
                frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_CONSTRUCTOR) :
                frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_REFLECT_METHOD);

        const auto retTypeArrayClass = frame.mem.getObjectArrayClass(*retTypeClass);
        
        const auto &methods = instanceMirrorClass->methods;
        std::vector<InstanceOop *> methodInstances;
        for (const auto &method : methods) {
            if (publicOnly && !method->isPublic()) {
                continue;
            }
            if (method->isClInit()) {
                continue;
            }
            if (isConstructor) {
                if (!method->isConstructor()) {
                    continue;
                }
            } else {
                if (method->isConstructor()) {
                    continue;
                }
            }

            const auto mirrorMethodInstance = method->getMirror(&frame);
            methodInstances.emplace_back(mirrorMethodInstance);
        }

        const auto retArrayOop = frame.mem.newObjArrayOop(retTypeArrayClass, methodInstances.size());
        for (size_t i = 0; i < methodInstances.size(); ++i) {
            retArrayOop->data[i] = methodInstances[i];
        }

        frame.returnRef(retArrayOop);
    }

    void getDeclaredConstructors0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        getDeclaredCommons0(frame, true);
    }

    void getDeclaredMethods0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        getDeclaredCommons0(frame, false);
    }

    void getInterfaces0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        frame.returnRef(nullptr);
    }

    //native void setSigners(Object[] signers);
    void setSigners(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
    }

    //native java.security.ProtectionDomain getProtectionDomain0();
    void getProtectionDomain0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        frame.returnRef(nullptr);
    }

    //native String getGenericSignature0();
    void getGenericSignature0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto signature = (CAST_INSTANCE_CLASS(mirrorClass))->getSignature();
        if (!signature.empty()) {
            frame.returnRef(frame.mem.getInternString(signature));
            return;
        }

        frame.returnRef(nullptr);
    }

    //native byte[] getRawAnnotations();
    void getRawAnnotations(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);

        if (instanceMirrorClass->basicAnnotationContainer != nullptr) {
            const auto byteArrayOop = 
                instanceMirrorClass
                    ->basicAnnotationContainer
                    ->runtimeVisibleAnnotation.createByteTypeArrayOop(frame);
            if (byteArrayOop != nullptr) {
                frame.returnRef(byteArrayOop);
                return;
            }
        }
        frame.returnRef(nullptr);
    }

    //native byte[] getRawTypeAnnotations(); 
    void getRawTypeAnnotations(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto mirrorClass = getMirrorClass(frame);
        if (mirrorClass == nullptr || mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        
        if (instanceMirrorClass->basicAnnotationContainer != nullptr) {
            const auto byteArrayOop = 
                instanceMirrorClass
                    ->basicAnnotationContainer
                    ->runtimeVisibleTypeAnnotation.createByteTypeArrayOop(frame);
            if (byteArrayOop != nullptr) {
                frame.returnRef(byteArrayOop);
                return;
            }
        }
        frame.returnRef(nullptr);
    }

    //native byte[] getTypeAnnotationBytes0();
    void fieldGetTypeAnnotationBytes0(Frame &frame) {
        const auto self = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(self)
        const auto slotId = self->getFieldValue("slot" "I").i4Val;
        const auto fieldClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(self->getFieldValue("clazz" "Ljava/lang/Class;").refVal)->getMirrorClass());
        const auto fieldPtr = fieldClass->fields[slotId].get();

        if (fieldPtr->basicAnnotationContainer != nullptr) {
            const auto byteArrayOop = 
                fieldPtr->basicAnnotationContainer
                        ->runtimeVisibleTypeAnnotation.createByteTypeArrayOop(frame);
            
            frame.returnRef(byteArrayOop);
            return;
        }

        frame.returnRef(nullptr);
    }

    //native ConstantPool getConstantPool();
    void getConstantPool(Frame &frame) {
        const auto instance = CAST_MIRROR_OOP(frame.getThisInstance());
        ASSERT_IF_NULL_THROW_NPE(instance)
        const auto instanceClass = instance->getClass();
        if (instanceClass == nullptr || instanceClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnRef(nullptr);
            return;
        }
        const auto mirrorClass = CAST_INSTANCE_CLASS(instance->getMirrorClass());

        const auto constantPoolPtr = mirrorClass->getConstantPoolMirror(&frame);
        frame.returnRef(constantPoolPtr);
    }

    //private native Class<?>[] getDeclaredClasses0();
    void getDeclaredClasses0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
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
                getConstantStringFromPoolByIndexInfo(
                    constantPool, 
                    innerClassesAttr->classes[i]->innerClassInfoIndex
                );
            retArrayOop->data[i] = frame.mem.getClass(innerClassName)->getMirror(&frame);
        }
            
        frame.returnRef(retArrayOop);
    }

    //private native Parameter[] getParameters0();
    void getParameters0(Frame &frame) {
        const auto method = frame.getThisInstance();
        ASSERT_IF_NULL_THROW_NPE(method);
        const auto methodClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(method->getFieldValue("clazz" "Ljava/lang/Class;").refVal)->getMirrorClass());
        const auto slotId = method->getFieldValue("slot" "I").i4Val;
        const auto methodPtr = methodClass->methods[slotId].get();
        const auto parameterClass = frame.mem.getInstanceClass("java/lang/reflect/Parameter");
        const auto result = frame.mem.newObjArrayOop(
            frame.mem.getObjectArrayClass(*parameterClass), 
            methodPtr->paramType.size()
        );

        for (size_t i = 0; i < methodPtr->paramType.size(); ++i) {
            const auto parameter = frame.mem.newInstance(parameterClass);
            const auto name = cformat("arg{}", i);
            parameter->setFieldValue("name" "Ljava/lang/String;", Slot(frame.mem.getInternString(name)));
            parameter->setFieldValue("modifiers" "I", Slot(CAST_I4(0)));
            parameter->setFieldValue("executable" "Ljava/lang/reflect/Executable;", Slot(method));
            parameter->setFieldValue("index" "I", Slot(CAST_I4(i)));
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
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass(frame));
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool[index].get();

        const auto val = CAST_CONSTANT_INTEGER_INFO(ptr)->value;
        frame.returnI4(val);
    }

    //native long     getLongAt0          (Object constantPoolOop, int index);
    void getLongAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass(frame));
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool[index].get();

        const auto val = CAST_CONSTANT_LONG_INFO(ptr)->value;
        frame.returnI8(val);
    }

    //native float    getFloatAt0         (Object constantPoolOop, int index);
    void getFloatAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass(frame));
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool[index].get();

        const auto val = CAST_CONSTANT_FLOAT_INFO(ptr)->value;
        frame.returnF4(val);
    }

    //native double   getDoubleAt0        (Object constantPoolOop, int index);
    void getDoubleAt0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass(frame));
        const auto &constantPool = mirrorClass->constantPool;
        const auto ptr = constantPool[index].get();

        const auto val = CAST_CONSTANT_DOUBLE_INFO(ptr)->value;
        frame.returnF8(val);
    }

    //native String   getUTF8At0          (Object constantPoolOop, int index);
    void getUTF8At0(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis());
        const auto index = frame.getLocalI4(2);
        const auto mirrorClass = CAST_INSTANCE_CLASS(getMirrorClass(frame));
        const auto &constantPool = mirrorClass->constantPool;

        const auto val = getConstantStringFromPool(constantPool, index);
        frame.returnRef(frame.mem.getInternString(val));
    }


    //static native Object invoke0(Method m, Object obj, Object[] args);
    void invoke0(Frame &frame) {
        const auto method = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        ASSERT_IF_NULL_THROW_NPE(method);
        const auto obj = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto args = CAST_OBJ_ARRAY_OOP(frame.getLocalRef(2));

        const auto methodClass = CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(method->getFieldValue("clazz" "Ljava/lang/Class;").refVal)->getMirrorClass());
        const auto slotId = method->getFieldValue("slot" "I").i4Val;
        const auto methodPtr = methodClass->methods[slotId].get();

        std::vector<Slot> params;
        if (!methodPtr->isStatic()) {
            ASSERT_IF_NULL_THROW_NPE(obj)
            params.emplace_back(obj);
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
        if (frame.markThrow) {
            return;
        }
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
        ASSERT_IF_NULL_THROW_NPE(instance);
        const auto mirrorClass = instance->getMirrorClass();
        frame.returnI4(CAST_I4(mirrorClass->getAccessFlags()));
    }

}


#endif