#include "thread.hpp"

#include <utility>
#include "vm.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "class_loader.hpp"
#include "key_slot_id.hpp"
#include "exception_helper.hpp"
#include "os_platform.hpp"

namespace RexVM {

    VMThreadMethod::VMThreadMethod(Method *method, std::vector<Slot> params) : method(method), params(std::move(params)) {
    }

    VMThreadMethod::VMThreadMethod(VMTheadNativeHandler nativeMethod) : nativeMethod(std::move(nativeMethod)) {}

    //Normal
    VMThread::VMThread(VM &vm, InstanceClass * const klass) :
            InstanceOop(klass),
            vm(vm),
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
        const auto method = getInstanceClass()->getMethod("run", "()V", false);
        const std::vector<Slot> params = { Slot(this) };
        runMethods.emplace_back(std::make_unique<VMThreadMethod>(method, params));
    }

    //Main
    VMThread::VMThread(VM &vm) : 
            InstanceOop(vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD)),
            vm(vm), 
            stackMemory(std::make_unique<Slot[]>(THREAD_STACK_SLOT_SIZE)),
            stackMemoryType(std::make_unique<SlotTypeEnum[]>(THREAD_STACK_SLOT_SIZE)) {
    }

    VMThread *VMThread::createOriginVMThread(VM &vm) {
        auto vmThread = new VMThread(vm);

        const auto threadGroupClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD_GROUP);
        const auto vmThreadGroup = vm.oopManager->newInstance(vmThread, threadGroupClass);

        vmThread->setFieldValue("group", "Ljava/lang/ThreadGroup;", Slot(vmThreadGroup));
        vmThread->setFieldValue("priority", "I", Slot(CAST_I8(1)));
        return vmThread;
    }

    VMThread::~VMThread() = default;

    void VMThread::setName(const RexVM::cstring &name) {
        threadName = name;
    }

    void VMThread::run() {
        if (!threadName.empty()) {
            setThreadName(threadName.c_str());
        }
        setStatus(ThreadStatusEnum::RUNNABLE);
        for (const auto &item: runMethods) {
            if (item->method != nullptr) {
                createFrameAndRunMethod(*this, *item->method, item->params, nullptr);
            } else if (item->nativeMethod != nullptr) {
                item->nativeMethod();
            }
        }
        setStatus(ThreadStatusEnum::TERMINATED);

        //vm.oopManager->defaultOopHolder.addAnotherHolderOops(oopHolder);
        cprintln("aaaa");

        std::lock_guard<std::recursive_mutex> lock(getAndInitMonitor()->monitorMtx);
        notify_all();
    }

    void VMThread::start(Frame *currentFrame_, bool addToThreadDeque) {
        if (getStatus() != ThreadStatusEnum::NEW) {
            throwIllegalThreadStateException(*currentFrame_);
        }

        if (addToThreadDeque) {
            vm.addStartThread(this);
        }

        nativeThread = std::thread([this]() {
            run();
        });
    }

    void VMThread::addMethod(Method *method, const std::vector<Slot>& params) {
        runMethods.emplace_back(std::make_unique<VMThreadMethod>(method, params));
    }

    void VMThread::addMethod(VMTheadNativeHandler &method) {
        runMethods.emplace_back(std::make_unique<VMThreadMethod>(method));
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
        if (nativeThread.joinable()) {
            nativeThread.join();
        }
    }

    void VMThread::getThreadGCRoots(std::vector<ref> &result) const {
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            cur->getLocalObjects(result);
            cur->operandStackContext.getStackObjects(result);
        }
    }

    void VMThread::getCollectRoots(std::vector<ref> &result) const {
        if (currentFrame == nullptr) {
            return;
        }

        const auto firstStackPtr = stackMemory.get();
        const auto lastStackPtr = currentFrame->operandStackContext.memory + currentFrame->operandStackContext.sp;

        for (auto cur = firstStackPtr; cur != (lastStackPtr + 1); ++cur) {
            const auto index = cur - firstStackPtr;
            const auto slot = *cur;
            const auto slotType = stackMemoryType[index];
            if (slotType == SlotTypeEnum::REF && slot.refVal != nullptr) {
                result.emplace_back(slot.refVal);
            }
        }
    }

    bool VMThread::hasNativeCall() const {
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            if (cur->nativeCall) {
                return true;
            }
        }
        return false;
    }




}