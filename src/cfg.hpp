#ifndef CFG_HPP
#define CFG_HPP
#include "config.hpp"
#include <memory>
#include <vector>

namespace RexVM {

    struct Method;

    struct MethodBlock {

        explicit MethodBlock(u4 index, u4 startPC, u4 endPC);

        u4 index;
        u4 length;
        u4 startPC;
        u4 endPC;
        bool autoJmp{false};

        std::vector<u4> jumpToBlockIndex;
        std::vector<u4> parentBlockIndex;
    };

    struct MethodCFG {

        explicit MethodCFG(const Method &method);

        const Method &method;

        std::vector<std::unique_ptr<MethodBlock>> blocks;

        //是否有向前jump 这种暂时不支持编译
        bool jumpFront{false};

        void build();

    };
}

#endif
