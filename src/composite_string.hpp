#ifndef COMPOSITE_STRING_HPP
#define COMPOSITE_STRING_HPP
#include "config.hpp"
#include "composite_ptr.hpp"

namespace RexVM {

    struct CompositeString : Composite<const cchar *, u2> {

        explicit CompositeString(const cchar *str);

    };

}

#endif
