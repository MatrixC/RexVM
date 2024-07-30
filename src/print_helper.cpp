#include "print_helper.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "basic_type.hpp"
#include "basic_java_class.hpp"
#include "string_pool.hpp"

namespace RexVM {

    void printAccessFlag(u2 accessFlag) {

    }

    void printBase(ClassFile &cf) {

    }

    void printConstant(ClassFile &cf) {
        for (auto i = 0; i < cf.constantPoolCount; ++i) {
            const auto &constantPtr = cf.constantPool.at(i);
            if (constantPtr != nullptr) {
                cprintln("{:>2} = {}", i, constantPtr->toString());
            }
        }
    }

    void printMethods(ClassFile &cf) {
        cprintln("method count {}", cf.methodCount);
        for (const auto &methodInfo: cf.methods) {
            cprintln("{}, {}, {}, {}", methodInfo->accessFlags, methodInfo->nameIndex, methodInfo->descriptorIndex,
                    methodInfo->attributesCount);
        }
    }

    void printCF(ClassFile &cf) {
        printMethods(cf);
    }

    void pClass(Class *klass) {
        switch (klass->type) {
            case ClassTypeEnum::PRIMITIVE_CLASS: {
                const auto clazz = CAST_PRIMITIVE_CLASS(klass);
                cprintln("className :{}", clazz->name);
                cprintln("type: PrimitiveClass, basicType: {}", getPrimitiveClassNameByBasicType(clazz->basicType));
                break;
            }

            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto clazz = CAST_INSTANCE_CLASS(klass);
                cprintln("className :{}", clazz->name);
                cprintln("type: InstanceClass");
                for (const auto& item : clazz->fields) {
                    cprintln("field name: {}, descritpro: {}", item->name, item->descriptor);
                }
                cprintln("");
                for (const auto &item : clazz->methods) {
                    cprintln("method name: {}, descritpro: {}", item->name, item->descriptor);
                }
                cprintln("");
                if (clazz->superClass != nullptr) {
                    cprintln("superClass: {}", clazz->superClass->name);
                }

                cprintln("");
                if (!clazz->interfaces.empty()) {
                    for (const auto &item : clazz->interfaces) {
                        cprintln("interface: {}", item->name);
                    }
                }
                
                cprintln("end");
                break;
            }

            case ClassTypeEnum::TYPE_ARRAY_CLASS: {
                const auto clazz = CAST_TYPE_ARRAY_CLASS(klass);
                cprintln("className :{}", clazz->name);
                cprintln("type: TypeArrayClass, basicType: {}", getPrimitiveClassNameByBasicType(clazz->elementType));
                break;
            }

            case ClassTypeEnum::OBJ_ARRAY_CLASS: {
                const auto clazz = CAST_OBJ_ARRAY_CLASS(klass);
                cprintln("className :{}", clazz->name); 
                cprintln("type: ObjArrayClass, elementClass: {}", clazz->elementClass->name);
                break;
            }
        }
    }

    void pObjArray(Oop *oop) {
        const auto clazz = CAST_OBJ_ARRAY_CLASS(oop->klass);
        const auto arrayOop = CAST_OBJ_ARRAY_OOP(oop);
        cprintln("className :{}, dataLength: {}", clazz->name, arrayOop->dataLength); 
        cprintln("type: ObjArrayClass, elementClass: {}", clazz->elementClass->name);
    }


    cstring formatSlot(Slot val, SlotTypeEnum type) {
        switch (type) {
            case SlotTypeEnum::I4:
                return cformat("I4({})", val.i4Val);
            case SlotTypeEnum::I8:
                return cformat("I8({})", val.i8Val);
            case SlotTypeEnum::F4:
                return cformat("F4({})", val.f4Val);
            case SlotTypeEnum::F8:
                return cformat("F8({})", val.f8Val);
            case SlotTypeEnum::REF: {
                const auto klass = val.refVal->klass;
                if (klass->name == JAVA_LANG_STRING_NAME) {
                    return cformat("String({})", StringPool::getJavaString(CAST_INSTANCE_OOP(val.refVal)));
                } else {
                    return cformat("ref class({})", val.refVal->klass->name);
                }
            }

            default:
                return cformat("None()", val.i8Val);
        }
    }

}