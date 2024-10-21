#ifndef STACK_HPP
#define STACK_HPP

#include <vector>
#include <tuple>
#include "../config.hpp"
#include "../basic_type.hpp"

namespace RexVM {

    struct Oop;

    struct StackContext {
        //栈顶指针 默认值为-1 始终指向最后一个插入的元素
        i4 sp;

        Slot *memory;
        SlotTypeEnum *memoryType;

        explicit StackContext(Slot *memory, SlotTypeEnum *memoryType, i4 pos);

        void push(Slot val, SlotTypeEnum slotType);
        void push(std::tuple<Slot, SlotTypeEnum> valWithType);

        Slot pop();
        [[nodiscard]] Slot top() const;
        std::tuple<Slot, SlotTypeEnum> popWithSlotType();
        [[nodiscard]] std::tuple<Slot, SlotTypeEnum> topWithSlotType() const;
        void pop(i4 size);

        void reset();

        [[nodiscard]] Slot getStackOffset(size_t offset) const;
        void getStackObjects(std::vector<ref> &result) const;

        [[nodiscard]] Slot *getCurrentSlotPtr() const;
        [[nodiscard]] SlotTypeEnum *getCurrentSlotTypePtr() const;

        void dup();
        void dup_x1();
        void dup_x2();
        void dup2();
        void dup2_x1();
        void dup2_x2();
        void swapTop();
    };
}

#endif
