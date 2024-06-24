#include "config.hpp"
#include "utils/binary.hpp"
#include "utils/class_utils.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "constant_info.hpp"
#include "constant_pool.hpp"
#include "vm.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "interpreter.hpp"

namespace RexVM {

    Thread::Thread(VM &vm) : vm(vm) {
        const auto &oopManager = vm.oopManager;
        vmThread = oopManager->newThreadOop(this);
    }

    Thread::~Thread() {
    }

    std::vector<Oop *> Thread::getThreadGCRoots() const {
        std::vector<Oop *> result;
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            const auto localObjects = cur->getLocalObjects();
            const auto stackObjects = cur->operandStackContext.getObjects();
            if (localObjects.size() > 0) {
                result.insert(result.end(), localObjects.begin(), localObjects.end());
            }
            if (stackObjects.size() > 0) {
                result.insert(result.end(), stackObjects.begin(), stackObjects.end());
            }
        }
        return result;
    }

    ThreadOop * Thread::getThreadMirror() const {
        return vmThread;
    }


}