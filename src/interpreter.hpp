#ifndef INTERPRETER_FIX_HPP
#define INTERPRETER_FIX_HPP
#include <array>
#include "config.hpp"

namespace RexVM {

    extern std::array<MethodHandler, 256> OpCodeHandlers;

}

#endif