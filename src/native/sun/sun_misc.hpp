#ifndef NATIVE_SUN_MISC_HPP
#define NATIVE_SUN_MISC_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../constant_pool.hpp"
#include "../../utils/string_utils.hpp"

namespace RexVM::Native::SUN::MISC {

    void getLookupCacheURLs(Frame &frame) {
        frame.returnRef(nullptr);
    }

}

#endif