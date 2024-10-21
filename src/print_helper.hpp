#ifndef PRINT_HELPER_HPP
#define PRINT_HELPER_HPP

#include "config.hpp"
#include <vector>
#include <tuple>

namespace RexVM {

    struct ClassFile;
    struct Class;
    struct ObjArrayOop;
    struct Oop;
    struct InstanceOop;
    struct Frame;

    void printConstant(ClassFile &cf);

    void printMethods(ClassFile &cf);

    void pClass(Class *klass);

    void pObjArray(Oop *oop);

    cstring formatArray(Frame &frame, ref oop);
    cstring formatSlot(Frame &frame, Slot val, SlotTypeEnum type);

    void pio(Frame &frame, ref oop);

    void pSlots(Frame &frame, std::vector<std::tuple<Slot, SlotTypeEnum>> &slots);

}

#endif