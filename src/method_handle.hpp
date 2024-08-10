#ifndef METHOD_HANDLE_HPP
#define METHOD_HANDLE_HPP
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

    constexpr i4 MN_IS_METHOD           = 0x00010000; // method (not constructor)
    constexpr i4 MN_IS_CONSTRUCTOR      = 0x00020000; // constructor
    constexpr i4 MN_IS_FIELD            = 0x00040000; // field
    constexpr i4 MN_IS_TYPE             = 0x00080000; // nested type
    constexpr i4 MN_CALLER_SENSITIVE    = 0x00100000; // @CallerSensitive annotation detected
    constexpr i4 MN_REFERENCE_KIND_SHIFT = 24; // refKind
    constexpr i4 MN_REFERENCE_KIND_MASK = 0x0F000000 >> MN_REFERENCE_KIND_SHIFT;
    constexpr i4 MN_SEARCH_SUPERCLASSES = 0x00100000;
    constexpr i4 MN_SEARCH_INTERFACES   = 0x00200000;
    const cstring JAVA_LANG_INVOKE_METHOD_TYPE_NAME = "java/lang/invoke/MethodType";

    inline const cstring METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR = "([Ljava/lang/Object;)Ljava/lang/Object;";

    bool isMethodHandleInvoke(const cstring &className, const cstring &memberName);

    bool isStaticMethodHandleType(MethodHandleEnum kind);

    void invokeDynamic(Frame &frame, u2 invokeDynamicIdx);

    cstring methodHandleGetDescriptor(Class *clazz, InstanceOop *type, const cstring &name);
}

#endif