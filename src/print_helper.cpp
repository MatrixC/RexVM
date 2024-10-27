#include "print_helper.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "frame.hpp"
#include "basic_type.hpp"
#include "string_pool.hpp"
#include "class_file.hpp"
#include "constant_info.hpp"


namespace RexVM {

    void printConstant(ClassFile &cf) {
        for (auto i = 0; i < cf.constantPoolCount; ++i) {
            const auto &constantPtr = cf.constantPool[i];
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
        switch (klass->getType()) {
            case ClassTypeEnum::PRIMITIVE_CLASS: {
                const auto clazz = CAST_PRIMITIVE_CLASS(klass);
                cprintln("PrimitiveClass({}): {}", getPrimitiveClassNameByBasicType(clazz->basicType), clazz->toView());
                break;
            }

            case ClassTypeEnum::INSTANCE_CLASS: {
                const auto clazz = CAST_INSTANCE_CLASS(klass);
                cprintln("InstanceClass: {}", clazz->toView());
                for (const auto& item : clazz->fields) {
                    cprintln("Field: {}", item->toView());
                }
                cprintln("");
                for (const auto &item : clazz->methods) {
                    cprintln("Method: {}", item->toView());
                }
                cprintln("");
                if (clazz->getSuperClass() != nullptr) {
                    cprintln("SuperClass: {}", clazz->getSuperClass()->toView());
                }
                cprintln("");
                FOR_FROM_ZERO(clazz->getInterfaceSize()) {
                    const auto item = clazz->getInterfaceByIndex(i);
                    cprintln("Interface: {}", item->toView());
                }
                break;
            }

            case ClassTypeEnum::TYPE_ARRAY_CLASS: {
                const auto clazz = CAST_TYPE_ARRAY_CLASS(klass);
                cprintln("TypeArrayClass({}): {}", getPrimitiveClassNameByBasicType(clazz->elementType), clazz->toView());
                break;
            }

            case ClassTypeEnum::OBJ_ARRAY_CLASS: {
                const auto clazz = CAST_OBJ_ARRAY_CLASS(klass);
                cprintln("TypeArrayClass({}): {}", clazz->elementClass->toView(), clazz->toView());
                break;
            }
        }
    }

    void pObjArray(ref oop) {
        const auto clazz = CAST_OBJ_ARRAY_CLASS(oop->getClass());
        const auto arrayOop = CAST_OBJ_ARRAY_OOP(oop);
        cprintln("className :{}, dataLength: {}", clazz->toView(), arrayOop->getDataLength());
        cprintln("type: ObjArrayClass, elementClass: {}", clazz->elementClass->toView());
    }

    cstring formatArray(Frame &frame, ref oop) {
        const auto klass = oop->getClass();
        if (!klass->isArray()) {
            panic("oop is not array type");
        }

        const auto arraysClass = frame.mem.getInstanceClass("java/util/Arrays");
        const auto arrayClassName = 
            klass->getType() == ClassTypeEnum::TYPE_ARRAY_CLASS ? 
                cformat("({})Ljava/lang/String;", klass->toView()) :
                "([Ljava/lang/Object;)Ljava/lang/String;";
        const auto toStringMethod = arraysClass->getMethod("toString", arrayClassName, true);
        auto [val, type] = frame.runMethodManual(*toStringMethod, { Slot(oop) });
        if (frame.markThrow) {
            return "";
        }
        const auto ret = val.refVal == nullptr ? "null" : VMStringHelper::getJavaString(CAST_INSTANCE_OOP(val.refVal));
        return cformat("Array: {} {}", klass->toView(), ret);
    }

    cstring formatInstance(Frame &frame, InstanceOop *oop) {
        const auto className = oop->getClass()->toView();
        const auto objectsClass = frame.mem.getInstanceClass("java/util/Objects");
        const auto toStringMethod = objectsClass->getMethod("toString" "(Ljava/lang/Object;)Ljava/lang/String;", true);
        auto [val, type] = frame.runMethodManual(*toStringMethod, { Slot(oop) });
        auto ret = val.refVal == nullptr ? "null" : VMStringHelper::getJavaString(CAST_INSTANCE_OOP(val.refVal));
        ret = ret.empty() ? "[EMPTY]" : ret;
        return cformat("{} {}", className, ret);
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
                const auto klass = refVal->getClass();
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
            cprintln("Field {}: {}", field->toView(), formatValue);
        }
    }

    void pSlots(Frame &frame, std::vector<std::tuple<Slot, SlotTypeEnum>> &slots) {
        for (const auto &item : slots) {
            const auto [val, type] = item;
            cprintln("{}", formatSlot(frame, val, type));
        }
    }

}