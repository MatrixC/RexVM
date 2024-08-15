#ifndef NATIVE_CORE_JAVA_LANG_RUNTIME_HPP
#define NATIVE_CORE_JAVA_LANG_RUNTIME_HPP
#include <thread>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../finalize.hpp"


namespace RexVM::Native::Core {

    void availableProcessors(Frame &frame) {
        frame.returnI4(CAST_I4(std::thread::hardware_concurrency()));
    }

    //native void gc();
    void _gc(Frame &frame) {
        std::thread gcThread([frame]() {
            frame.vm.collector->startGC();
        });
        gcThread.detach();

        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }

}


#endif