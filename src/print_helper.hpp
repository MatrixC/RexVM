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

    void printAccessFlag(u2 accessFlag);

    void printBase(ClassFile &cf);

    void printConstant(ClassFile &cf);

    void printMethods(ClassFile &cf);

    void printCF(ClassFile &cf);

    void pClass(Class *klass);

    void pObjArray(Oop *oop);

    cstring formatSlot(Slot val, SlotTypeEnum type);

}

#endif