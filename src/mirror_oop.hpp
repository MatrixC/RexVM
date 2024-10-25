#ifndef MIRROR_OOP_HPP
#define MIRROR_OOP_HPP
#include "basic.hpp"
#include "composite_ptr.hpp"
#include "oop.hpp"

namespace RexVM {

    struct InstanceClass;
    struct Class;
    struct Method;
    struct Field;

    struct MirOop : InstanceOop {
        Composite<voidPtr, u2> mirror;
#ifdef DEBUG
        cstring mirrorName;
#endif

        explicit MirOop(InstanceClass *klass, voidPtr mirror, MirrorObjectTypeEnum type);
        ~MirOop();

        void clearHolder();

        [[nodiscard]] MirrorObjectTypeEnum getMirrorObjectType() const;

        [[nodiscard]] Class *getMirrorClass() const;
        [[nodiscard]] Method *getMirrorMethod() const;
        [[nodiscard]] Field *getMirrorField() const;
        [[nodiscard]] Method *getMemberNameMethod();
    };

}

#endif
