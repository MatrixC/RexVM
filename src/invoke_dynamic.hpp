#ifndef INVOKE_DYNAMIC__HPP
#define INVOKE_DYNAMIC__HPP
#include "config.hpp"
#include <vector>
#include <memory>

#include "vm.hpp"
#include "class.hpp"
#include "attribute_info.hpp"
#include "constant_info.hpp"

namespace RexVM {

    enum class MethodHandleEnum : u1 {
        REF_getField = 1,
        REF_getStatic = 2,
        REF_putField = 3,
        REF_putStatic = 4,
        REF_invokeVirtual = 5,
        REF_invokeStatic = 6,
        REF_invokeSpecial = 7,
        REF_newInvokeSpecial = 8,
        REF_invokeInterface = 9,
    };


    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx);
    InstanceOop *createLookup(VM &vm, Frame &frame);
    InstanceOop *createMethodHandle(Frame &frame, ConstantMethodHandleInfo *methodHandleInfo);
    InstanceOop *createMethodHandle(Frame &frame, MethodHandleEnum kind, const cstring &className, const cstring &methodHandleMemberName, const cstring &methodHandleMemberDescriptor);


}

#endif