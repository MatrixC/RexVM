#ifndef CFG_HPP
#define CFG_HPP
#include "config.hpp"
#include <memory>
#include <vector>

namespace RexVM {

    struct Method;

    struct MethodBlock {

        explicit MethodBlock(const Method &method, u4 startPC, u4 endPC);

        u4 length;
        u4 startPC;
        u4 endPC;

        std::vector<MethodBlock *> jumpTo;
    };

    struct MethodCFG {

        explicit MethodCFG(const Method &method);

        const Method &method;

        std::vector<std::unique_ptr<MethodBlock>> blocks;

        void build();

    };
}

#endif
