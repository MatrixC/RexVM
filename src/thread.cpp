#include "thread.hpp"
#include "oop.hpp"
#include "vm.hpp"
#include "frame.hpp"
#include "memory.hpp"

namespace RexVM {

    Thread::Thread(VM &vm, const cstring &name) : vm(vm), name(name) {
        const auto &oopManager = vm.oopManager;
        vmThread = oopManager->newThreadOop(this);
    }

    Thread::~Thread() = default;

    std::vector<Oop *> Thread::getThreadGCRoots() const {
        std::vector<Oop *> result;
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            const auto localObjects = cur->getLocalObjects();
            const auto stackObjects = cur->operandStackContext.getObjects();
            if (!localObjects.empty()) {
                result.insert(result.end(), localObjects.begin(), localObjects.end());
            }
            if (!stackObjects.empty()) {
                result.insert(result.end(), stackObjects.begin(), stackObjects.end());
            }
        }
        return result;
    }

    ThreadOop * Thread::getThreadMirror() const {
        return vmThread;
    }


}