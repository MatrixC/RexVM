#ifndef MIRROR_BASE_HPP
#define MIRROR_BASE_HPP
#include "config.hpp"

namespace RexVM {

    struct SpinLock;
    struct MirOop;
    struct Frame;
    struct MirOop;

    using MirrorInitHandler = void (*)(MirOop *);

    struct MirrorBase {
        MirOop *mirOop{nullptr};

        explicit MirrorBase();
        ~MirrorBase();

        [[nodiscard]] MirOop *getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, SpinLock &lock, bool init, MirrorInitHandler initHandler);
    };
}

#endif