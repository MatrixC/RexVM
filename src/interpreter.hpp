#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP
#include <array>
#include "config.hpp"

namespace RexVM {

    extern std::array<MethodHandler, 256> OpCodeHandlers;

}

#endif