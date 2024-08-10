#ifndef NATIVE_CORE_JAVA_LANG_RUNTIME_HPP
#define NATIVE_CORE_JAVA_LANG_RUNTIME_HPP

#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include <thread>


namespace RexVM::Native::Core {

    void availableProcessors(Frame &frame) {
        frame.returnI4(CAST_I4(std::thread::hardware_concurrency()));
    }

}


#endif