#ifndef KEY_SLOT_ID_HPP
#define KEY_SLOT_ID_HPP
#include "basic.hpp"

namespace RexVM {

    extern u2 stringClassValueFieldSlotId;

    extern u2 throwableClassDetailMessageFieldSlotId;
    extern u2 throwableClassBacktraceFID;
    extern u2 throwableClassStacktraceFID;

    extern u2 threadClassThreadStatusFieldSlotId;
    extern u2 threadClassExitMethodSlotId;
    extern u2 threadClassDeamonFieldSlotId;
    extern u2 threadClassNameFieldSlotId;

    extern u2 steClassDeclaringClassFId;
    extern u2 steClassMethodNameFId;
    extern u2 steClassFileNameFId;
    extern u2 steClassLineNumberFId;

    extern u2 objClassCloneMID;
    extern u2 objClassGetClassMID;;
}

#endif
