#include "thread.hpp"

#include <utility>
#include "vm.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "class_loader.hpp"
#include "key_slot_id.hpp"
#include "exception_helper.hpp"
#include "memory.hpp"

namespace RexVM {

    //Normal
    VMThread::VMThread(VM &vm, InstanceClass * const klass) : 
            InstanceOop(klass), 
            vm(vm), 
            runMethod(getInstanceClass()->getMethod("run", "()V", false)),
            params(std::vector<Slot>{ Slot(this) }),
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
    }

    //Main
    VMThread::VMThread(VM &vm) : 
            InstanceOop(vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD)),
            vm(vm), 
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
    }

    VMThread::~VMThread() = default;

    void VMThread::reset(Method *method, std::vector<Slot> runParams) {
        setStatus(ThreadStatusEnum::NEW);
        runMethod = method;
        params = runParams;
    }

    void VMThread::run() {
        setStatus(ThreadStatusEnum::RUNNABLE);
        createFrameAndRunMethod(*this, *runMethod, params, nullptr);
        setStatus(ThreadStatusEnum::TERMINATED);
        std::lock_guard<std::recursive_mutex> lock(getAndInitMonitor()->monitorMtx);
        notify_all();
    }

    void VMThread::start(Frame *currentFrame) {
        if (getStatus() != ThreadStatusEnum::NEW) {
            throwIllegalThreadStateException(*currentFrame);
        }
        nativeThread = std::thread([this]() {
            run();
        }); 
    }

    cstring VMThread::getName() const {
        return "Thread";
    }

    void VMThread::setStatus(ThreadStatusEnum status) {
        setFieldValue(threadClassThreadStatusFieldSlotId, Slot(CAST_I4(status)));
    }

    ThreadStatusEnum VMThread::getStatus() const {
        return static_cast<ThreadStatusEnum>(getFieldValue(threadClassThreadStatusFieldSlotId).i4Val);
    }

    bool VMThread::isAlive() const {
        const auto status = getStatus();
        return status != ThreadStatusEnum::NEW && status != ThreadStatusEnum::TERMINATED;
    }

    void VMThread::join() {
        //if (!isMainThread && nativeThread.joinable()) {
        if (nativeThread.joinable()) {
            nativeThread.join();
        }
    }

    void VMThread::getThreadGCRoots(std::vector<ref> &result) const {
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            cur->getLocalObjects(result);
            cur->operandStackContext.getObjects(result);
        }
    }




}