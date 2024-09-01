#include "thread.hpp"

#include <utility>
#include "vm.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "memory.hpp"
#include "execute.hpp"
#include "string_pool.hpp"
#include "class_loader.hpp"
#include "key_slot_id.hpp"
#include "exception_helper.hpp"
#include "os_platform.hpp"
#include "utils/string_utils.hpp"

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
        if (klass->getClassName() == "java/lang/ref/Reference$ReferenceHandler") {
            //disable ReferenceHandler
            //这个方法一直在wait 不好让它进入safepoint
            return;
        }

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
        const auto &stringPool = vm.stringPool;

        const auto threadGroupClass = vm.bootstrapClassLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD_GROUP);
        const auto vmThreadGroup = vm.oopManager->newInstance(vmThread, threadGroupClass);
        vmThreadGroup->setFieldValue("name", "Ljava/lang/String;", Slot(stringPool->getInternString(vmThread, "system")));
        vmThreadGroup->setFieldValue("maxPriority", "I", Slot(CAST_I4(10)));

        vmThread->setFieldValue("group", "Ljava/lang/ThreadGroup;", Slot(vmThreadGroup));
        vmThread->setFieldValue("priority", "I", Slot(CAST_I8(1)));
        return vmThread;
    }

    VMThread::~VMThread() = default;

    void VMThread::setName(const cstring &name) {
#ifdef DEBUG
        threadName = name;
#endif
        const auto &stringPool = vm.stringPool;
        setFieldValue(threadClassNameFieldSlotId, Slot(Slot(stringPool->getInternString(this, name))));
    }

    cstring VMThread::getName() const {
        const auto nameOop = getFieldValue(threadClassNameFieldSlotId).refVal;
        if (nameOop == nullptr) {
            return {};
        }
        return VMStringHelper::getJavaString(CAST_INSTANCE_OOP(nameOop));
    }

    void VMThread::run() {
        const auto threadName = getName();
        setThreadName(threadName.c_str());

        setStatus(ThreadStatusEnum::RUNNABLE);
        for (const auto &item: runMethods) {
            if (item->method != nullptr) {
                createFrameAndRunMethod(*this, *item->method, nullptr, item->params);
            } else if (item->nativeMethod != nullptr) {
                item->nativeMethod();
            }
        }

        const auto exitMethod = 
            vm.bootstrapClassLoader
                ->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THREAD)
                ->methods[threadClassExitMethodSlotId].get();

        createFrameAndRunMethod(*this, *exitMethod, nullptr, {Slot(this)});

        setStatus(ThreadStatusEnum::TERMINATED);
        std::lock_guard<std::recursive_mutex> lock(getAndInitMonitor()->monitorMtx);
        notify_all();
    }

    void VMThread::start(Frame *currentFrame_, bool userThread) {
        if (getStatus() != ThreadStatusEnum::NEW) {
            throwIllegalThreadStateException(*currentFrame_);
        }

        vm.threadManager->addThread(this, userThread);

        nativeThread = std::thread([this]() {
            run();
        });

        if (isDaemon()) {
            nativeThread.detach();
        }
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

    bool VMThread::isDaemon() const {
        return getFieldValue(threadClassDeamonFieldSlotId).i4Val != 0;
    }

    bool VMThread::isAlive() const {
        const auto status = getStatus();
        return status != ThreadStatusEnum::NEW && status != ThreadStatusEnum::TERMINATED;
    }

    void VMThread::setDaemon(bool on) {
        setFieldValue("daemon", "Z", on ? Slot(CAST_I4(1)) : ZERO_SLOT);
    }

    void VMThread::join() {
        if (nativeThread.joinable()) {
            nativeThread.join();
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

    void VMThread::getCollectRootsBak(std::vector<ref> &result) const {
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            cur->getLocalObjects(result);
            cur->operandStackContext.getStackObjects(result);
        }
    }

    void VMThread::setGCSafe(bool val) {
        gcSafe = val;
    }

    [[nodiscard]] bool VMThread::isGCSafe() const {
        return gcSafe;
    }


    ThreadManager::ThreadManager(VM &vm) : vm(vm) {
    }

    void ThreadManager::addThread(VMThread *thread, bool userThread) {
        if (userThread) {
            std::lock_guard<std::mutex> lock(joinListMtx);
            joinThreads.emplace(thread);
        }

        {
            std::lock_guard<std::mutex> lock(threadsMtx);
            threads.emplace_back(thread);
        }
    }

    void ThreadManager::joinUserThreads() {
        while (true) {
            VMThread *vmThread;
            {
                std::lock_guard<std::mutex> lock(joinListMtx);
                if (joinThreads.empty()) {
                    break;
                }
                vmThread = joinThreads.front();
                joinThreads.pop();
            }
            vmThread->join();
        }
    }

    bool ThreadManager::checkAllThreadStopForCollect() {
        std::lock_guard<std::mutex> lock(threadsMtx);
        for (const auto &item : threads) {
            if (item->getStatus() != ThreadStatusEnum::TERMINATED && !item->stopForCollect) {
                return false;
            }
        }
        return true;
    }

    std::vector<VMThread *> ThreadManager::getThreads() {
        std::lock_guard<std::mutex> lock(threadsMtx);
        std::vector<VMThread *> copyThreads(threads);
        return copyThreads;
    }

    ThreadSafeGuard::ThreadSafeGuard(VMThread &thread) : thread(thread) {
        thread.setGCSafe(false);
    }

    ThreadSafeGuard::~ThreadSafeGuard() {
        thread.setGCSafe(true);
    }




}