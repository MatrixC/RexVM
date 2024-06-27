//
// Created by ch on 12/31/23.
//

#ifndef STACK_HPP
#define STACK_HPP

#include <memory>
#include <vector>
#include <stack>
#include <memory>
#include "../config.hpp"
#include "../basic_type.hpp"

namespace RexVM {

    struct SlotWithType {
        Slot slot{0};
        SlotTypeEnum type;
    };

    struct Oop;

    struct StackContext {
        Slot *memory;
        i4 size;
        i4 sp;
        bool useSelfMemory{false};

        std::unique_ptr<SlotWithType[]> selfMemory;

        explicit StackContext(Slot *memory, i4 pos);
        explicit StackContext(i4 size, i4 pos);

        void push(Slot val, SlotTypeEnum slotType);

        Slot pop();

        void reset();

        [[nodiscard]] Slot getStackOffset(size_t offset) const;
        [[nodiscard]] std::vector<Oop *> getObjects() const;
    };
}

#endif
