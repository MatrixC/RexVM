#include "print_helper.hpp"
#include "class.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "frame.hpp"
#include "basic_type.hpp"
#include "basic_java_class.hpp"
#include "string_pool.hpp"

namespace RexVM {

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

    void pClass(Class *klass) {
        switch (klass->type) {
            case ClassTypeEnum::PRIMITIVE_CLASS: {
                const auto clazz = CAST_PRIMITIVE_CLASS(klass);
                cprintln("PrimitiveClass({}): {}", getPrimitiveClassNameByBasicType(clazz->basicType), clazz->name);
                break;
            }

            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto clazz = CAST_INSTANCE_CLASS(klass);
                cprintln("InstanceClass: {}", clazz->name);
                for (const auto& item : clazz->fields) {
                    cprintln("Field: {}, descritpro: {}", item->name, item->descriptor);
                }
                cprintln("");
                for (const auto &item : clazz->methods) {
                    cprintln("Method: {}, descritpro: {}", item->name, item->descriptor);
                }
                cprintln("");
                if (clazz->superClass != nullptr) {
                    cprintln("SuperClass: {}", clazz->superClass->name);
                }
                cprintln("");
                if (!clazz->interfaces.empty()) {
                    for (const auto &item : clazz->interfaces) {
                        cprintln("Interface: {}", item->name);
                    }
                }
                break;
            }

            case ClassTypeEnum::TYPE_ARRAY_CLASS: {
                const auto clazz = CAST_TYPE_ARRAY_CLASS(klass);
                cprintln("TypeArrayClass({}): {}", getPrimitiveClassNameByBasicType(clazz->elementType), clazz->name);
                break;
            }

            case ClassTypeEnum::OBJ_ARRAY_CLASS: {
                const auto clazz = CAST_OBJ_ARRAY_CLASS(klass);
                cprintln("TypeArrayClass({}): {}", clazz->elementClass->name, clazz->name);
                break;
            }
        }
    }

    void pObjArray(ref oop) {
        const auto clazz = CAST_OBJ_ARRAY_CLASS(oop->klass);
        const auto arrayOop = CAST_OBJ_ARRAY_OOP(oop);
        cprintln("className :{}, dataLength: {}", clazz->name, arrayOop->dataLength); 
        cprintln("type: ObjArrayClass, elementClass: {}", clazz->elementClass->name);
    }

    cstring formatArray(Frame &frame, ref oop) {
        const auto klass = oop->klass;
        if (!klass->isArray()) {
            panic("oop is not array type");
        }

        const auto arraysClass = frame.getCurrentClassLoader()->getInstanceClass("java/util/Arrays");
        const auto arrayClassName = 
            klass->type == ClassTypeEnum::TYPE_ARRAY_CLASS ? 
                cformat("({})Ljava/lang/String;", klass->name) :
                "([Ljava/lang/Object;)Ljava/lang/String;";
        const auto toStringMethod = arraysClass->getMethod("toString", arrayClassName, true);
        auto [val, type] = frame.runMethodManual(*toStringMethod, { Slot(oop) });
        const auto ret = val.refVal == nullptr ? "null" : StringPool::getJavaString(CAST_INSTANCE_OOP(val.refVal));
        return "Array:" + klass->name + " " + ret;
    }

    cstring formatInstance(Frame &frame, InstanceOop *oop) {
        const auto className = oop->klass->name;
        const auto objectsClass = frame.getCurrentClassLoader()->getInstanceClass("java/util/Objects");
        const auto toStringMethod = objectsClass->getMethod("toString", "(Ljava/lang/Object;)Ljava/lang/String;", true);
        auto [val, type] = frame.runMethodManual(*toStringMethod, { Slot(oop) });
        auto ret = val.refVal == nullptr ? "null" : StringPool::getJavaString(CAST_INSTANCE_OOP(val.refVal));
        ret = ret.empty() ? "[EMPTY]" : ret;
        return className + " " + ret;
    }

    cstring formatSlot(Frame &frame, Slot val, SlotTypeEnum type) {
        switch (type) {
            case SlotTypeEnum::I4:
                return cformat("I4 {}", val.i4Val);
            case SlotTypeEnum::I8:
                return cformat("I8 {}", val.i8Val);
            case SlotTypeEnum::F4:
                return cformat("F4 {}", val.f4Val);
            case SlotTypeEnum::F8:
                return cformat("F8 {}", val.f8Val);
            case SlotTypeEnum::REF: {
                const auto refVal = val.refVal;
                if (refVal == nullptr) {
                    return "null";
                }
                const auto klass = refVal->klass;
                if (klass->isArray()) {
                    return formatArray(frame, refVal);
                } else {
                    return formatInstance(frame, CAST_INSTANCE_OOP(refVal));
                }
            }
            default:
                return cformat("None()", val.i8Val);
        }
    }

    void pio(Frame &frame, ref refOop) {
        const auto oop = CAST_INSTANCE_OOP(refOop);
        const auto klass = oop->getInstanceClass();
        for (const auto &field : klass->fields) {
            const auto fieldType = field->getFieldSlotType();
            const auto fieldValue = 
                field->isStatic() ?
                klass->getFieldValue(field->slotId) :
                oop->getFieldValue(field->slotId);
            const auto formatValue = formatSlot(frame, fieldValue, fieldType);
            cprintln("Field {}: {}", field->name, formatValue);
        }
    }

    void pSlots(Frame &frame, std::vector<std::tuple<Slot, SlotTypeEnum>> &slots) {
        for (const auto &item : slots) {
            const auto [val, type] = item;
            cprintln("{}", formatSlot(frame, val, type));
        }
    }

}