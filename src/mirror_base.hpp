#ifndef MIRROR_BASE_HPP
#define MIRROR_BASE_HPP
#include "config.hpp"

namespace RexVM {

    struct SpinLock;
    struct MirOop;
    struct Frame;
    struct Class;
    struct Method;
    struct Field;

    struct MirrorBase {
        MirOop *mirOop{nullptr};

        explicit MirrorBase();
        ~MirrorBase();

        [[nodiscard]] MirOop *getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, voidPtr mirrorObj, SpinLock &lock, bool init);

        void clear(voidPtr checkPtr);

        void initClassMirrorOop(Frame &frame, Class *klass) const;
        void initMethodMirrorOop(Frame &frame, Method *method, bool isConstructor) const;
        void initFieldMirrorOop(Frame &frame, Field *field) const;
        void initConstantPoolMirrorOop(Frame &frame, Class *klass) const;
    };
}

#endif