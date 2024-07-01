#include "thread.hpp"

#include <utility>
#include "oop.hpp"
#include "vm.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "execute.hpp"

namespace RexVM {

    Thread::Thread(VM &vm, Method &method, std::vector<Slot> params, bool mainThread)
            : vm(vm),
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
        const auto &oopManager = vm.oopManager;
        vmThreadOop = oopManager->newThreadOop(this);

        if (mainThread) {
            status = ThreadStatusEnum::Running;
            createFrameAndRunMethod(*this, method, std::move(params), nullptr);
            status = ThreadStatusEnum::Terminated;
        } else {
            nativeThread = std::thread([&vm, &method, params = std::move(params), this]() {
                status = ThreadStatusEnum::Running;
                createFrameAndRunMethod(*this, method, std::move(params), nullptr);
                status = ThreadStatusEnum::Terminated;
            });
        }
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
        return vmThreadOop;
    }


}