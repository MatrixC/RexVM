#ifndef INVOKE_DYNAMIC_FIX_HPP
#define INVOKE_DYNAMIC_FIX_HPP
#include "config.hpp"
#include <vector>
#include <memory>

#include "vm.hpp"
#include "class.hpp"
#include "attribute_info.hpp"
#include "constant_info.hpp"

namespace RexVM {

    void invokeDynmic_(Frame &frame, u2 invokeDynamicIdx);
}

#endif