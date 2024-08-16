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

    using MirrorInitHandler = void (*)(MirOop *);

    struct MirrorBase {
        MirOop *mirOop{nullptr};

        explicit MirrorBase();
        ~MirrorBase();

        [[nodiscard]] MirOop *getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, voidPtr mirrorObj, SpinLock &lock, bool init);

        void clear(voidPtr checkPtr);

        void initClassMirrorOop(Frame &frame, Class *klass);
        void initMethodMirrorOop(Frame &frame, Method *method, bool isConstructor);
        void initFieldMirrorOop(Frame &frame, Field *field);
        void initConstantPoolMirrorOop(Frame &frame, Class *klass);
    };
}

#endif