#ifndef PRINT_HELPER_HPP
#define PRINT_HELPER_HPP

#include "utils/format.hpp"
#include "constant_info.hpp"
#include "attribute_info.hpp"
#include "class_file.hpp"


namespace RexVM {

    struct Class;
    struct ObjArrayOop;
    struct Oop;

    void printConstant(ClassFile &cf);

    void printMethods(ClassFile &cf);

    void pClass(Class *klass);

    void pObjArray(Oop *oop);

    cstring formatSlot(Frame &frame, Slot val, SlotTypeEnum type);

}

#endif