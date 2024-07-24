#include "class_file_print.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "basic_type.hpp"

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
            case ClassTypeEnum::PrimitiveClass: {
                const auto clazz = CAST_PRIMITIVE_CLASS(klass);
                cprintln("className :{}", clazz->name);
                cprintln("type: PrimitiveClass, basicType: {}", basicTypeClassName(clazz->basicType));
                break;
            }

            case ClassTypeEnum::InstanceClass: {
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

            case ClassTypeEnum::TypeArrayClass: {
                const auto clazz = CAST_TYPE_ARRAY_CLASS(klass);
                cprintln("className :{}", clazz->name);
                cprintln("type: TypeArrayClass, basicType: {}", basicTypeClassName(clazz->elementType));
                break;
            }

            case ClassTypeEnum::ObjArrayClass: {
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

}