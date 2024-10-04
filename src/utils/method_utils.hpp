#ifndef METHOD_UTILS_HPP
#define METHOD_UTILS_HPP
#include "../config.hpp"
#include <memory>
#include <vector>


namespace RexVM {

    struct Method;

    std::vector<std::tuple<u4, u4> > scanMethodLabel(const Method &method);
}

#endif
