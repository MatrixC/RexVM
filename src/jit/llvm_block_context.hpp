#ifndef LLVM_BLOCK_CONTEXT_HPP
#define LLVM_BLOCK_CONTEXT_HPP
#include "../config.hpp"
#include <stack>
#include <vector>
#include <memory>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "../opcode.hpp"
#include "../cfg.hpp"



namespace RexVM {

    struct Method;
    struct MethodCompiler;
    struct ByteReader;
    struct BlockContext;

    struct PassStack {
        BlockContext &blockContext;
        std::stack<llvm::Value *> remainValue;

        explicit PassStack(BlockContext &blockContext);
    };

    struct BlockContext {
        MethodCompiler &methodCompiler;
        llvm::IRBuilder<> &irBuilder;

        MethodBlock *methodBlock;
        //起始块
        llvm::BasicBlock *basicBlock;
        //最后块
        llvm::BasicBlock *lastBasicBlock;

        //从上个块传下来的StackValue
        std::vector<std::unique_ptr<PassStack>> passStack;
        //当前块的StackValue
        std::stack<llvm::Value *> blockValueStack;

        //当前块的本地变量表
        // std::vector<llvm::Value *> localVariableTable;
        std::unique_ptr<llvm::Value *[]> localVariableTable;

        std::vector<BlockContext *> jumpToBlocks;
        std::vector<BlockContext *> parentBlocks;

        explicit BlockContext(MethodCompiler &methodCompiler, MethodBlock *methodBlock);

        void initPassStack();

        void initLocalVariableTable();

        u4 pc{};

        void pushValue(llvm::Value *value);

        void pushValue(llvm::Value *value, SlotTypeEnum type);

        llvm::Value *popValue();

        llvm::Value *popValue(SlotTypeEnum type);

        void pushWideValue(llvm::Value *value);

        llvm::Value *popWideValue();

        llvm::Value *topValue();

        void padding();

        void unPadding();

        void pushI4Const(i4 value);

        void pushI8Const(i8 value);

        void pushF4Const(f4 value);

        void pushF8Const(f8 value);

        void pushNullConst();

        void processInstruction(OpCodeEnum opCode, ByteReader &byteReader);

        void compile();
    };

}

#endif
