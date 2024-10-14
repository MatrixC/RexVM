#ifndef CFG_HPP
#define CFG_HPP
#include "config.hpp"
#include <vector>

namespace RexVM {

    struct Method;

    struct MethodBlock {

        explicit MethodBlock(u4 index, u4 startPC, u4 endPC, u4 lastPC);

        u4 index{}; //第几个块 从0开始
        u4 length{};
        u4 startPC{}; //第一个opCode PC
        u4 endPC{}; //下一个块第一个opCode PC 所以是左闭右开[startPC, endPC)
        u4 lastPC{}; //当前块的最后一个opCode PC

        //如果一段无跳转的opCode中 中间某个指令被跳转 则会在那里形成新块
        //但是它上面的一条指令没有显式的做跳转 所以需要标记autoJmp 在生成ir时补上一条跳转指令
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
