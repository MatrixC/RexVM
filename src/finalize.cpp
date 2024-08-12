#include "finalize.hpp"
#include "vm.hpp"

namespace RexVM {

    Collector::Collector(VM &vm) : vm(vm) {
    }

    void Collector::stopTheWorld() {
        std::lock_guard<std::mutex> lock(mtx);
        markCollect = true;

        while(vm.getActiveThreadCount() != stopThreadCount) {
        }
    }

    void Collector::collectFinish() {
        stopThreadCount = 0;
        markCollect = false;
        cv.notify_all();
    }

    void Collector::checkStop() {
        if (!markCollect) [[likely]] {
            return;
        }

        ++stopThreadCount;
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !markCollect; });
    }

    void Collector::startGC() {
        stopTheWorld();
        //
        collectFinish();
    }
    

}