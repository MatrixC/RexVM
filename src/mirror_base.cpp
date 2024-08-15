#include "mirror_base.hpp"
#include "oop.hpp"
#include "frame.hpp"
#include "utils/spin_lock.hpp"

namespace RexVM {

    MirOop *MirrorBase::getBaseMirror(Frame *frame, MirrorObjectTypeEnum type, SpinLock &lock, bool init, MirrorInitHandler initHandler) {
        if (mirOop == nullptr) [[unlikely]] {
            if (!init) [[unlikely]] {
                return nullptr;
            }
            lock.lock();
            if (mirOop == nullptr) {
                if (frame != nullptr) {
                    mirOop = frame->mem.newMirror(nullptr, this, type);
                    if (initHandler != nullptr) {
                        initHandler(mirOop);
                    }
                }
            }
            lock.unlock();
        }
        return mirOop;
    }

    MirrorBase::MirrorBase() = default;

    MirrorBase::~MirrorBase() {
        delete mirOop;
    }

}