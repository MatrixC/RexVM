#include "thread.hpp"

#include <utility>
#include "vm.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "class_loader.hpp"
#include "basic_java_class.hpp"
#include "key_slot_id.hpp"

namespace RexVM {

    VMThread::VMThread(VM &vm, InstanceClass * const klass, Method *runnableMethod, std::vector<Slot> runnableMethodParams)
            : InstanceOop(klass), 
            vm(vm), 
            isMainThread(runnableMethod != nullptr), 
            runMethod(isMainThread ? *runnableMethod : *getInstanceClass()->getMethod("run", "()V", false)),
            params(isMainThread ? std::move(runnableMethodParams) : std::vector{ Slot(this) }),
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
    }

    VMThread::~VMThread() = default;

    void VMThread::run() {
        setStatus(ThreadStatusEnum::RUNNABLE);
        createFrameAndRunMethod(*this, runMethod, params, nullptr);
        setStatus(ThreadStatusEnum::TERMINATED);
        std::lock_guard<std::recursive_mutex> lock(monitorMtx);
        monitorCv.notify_all();
    }

    void VMThread::start() {
        if (getStatus() != ThreadStatusEnum::NEW) {
            panic("Thread status is not NEW");
            //TODO to java exception
        }
        if (isMainThread) [[unlikely]] {
            vm.addStartThread(this);
            run();
        } else {
            nativeThread = std::thread([this]() {
                run();
            }); 
            vm.addStartThread(this);
        }
    }

    cstring VMThread::getName() const {
        return "Thread";
    }

    void VMThread::setStatus(ThreadStatusEnum status) {
        setFieldValue(threadClassThreadStatusFieldSlotId, Slot(static_cast<i4>(status)));
    }

    ThreadStatusEnum VMThread::getStatus() const {
        return static_cast<ThreadStatusEnum>(getFieldValue(threadClassThreadStatusFieldSlotId).i4Val);
    }

    bool VMThread::isAlive() const {
        return getStatus() != ThreadStatusEnum::TERMINATED;
    }

    void VMThread::join() {
        if (!isMainThread && nativeThread.joinable()) {
            nativeThread.join();
        }
    }

    std::vector<Oop *> VMThread::getThreadGCRoots() const {
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




}