#include "stack.hpp"
#include "../oop.hpp"

namespace RexVM {

    StackContext::StackContext(Slot *memory, SlotTypeEnum *memoryType, i4 pos) :
        memory(memory), memoryType(memoryType), sp(pos) {
    }

    void StackContext::push(Slot val, SlotTypeEnum slotType) {
        ++sp;
        memory[sp] = val;
        memoryType[sp] = slotType;
    }

    void StackContext::push(std::tuple<Slot, SlotTypeEnum> valWithType) {
        const auto [val, type] = valWithType;
        push(val, type);
    }

    Slot StackContext::pop() {
        return memory[sp--];
    }

    Slot StackContext::top() const {
        return memory[sp];
    }

    std::tuple<Slot, SlotTypeEnum> StackContext::popWithSlotType() {
       const auto val = std::make_tuple(memory[sp], memoryType[sp]);
       sp--;
       return val;
    }

    std::tuple<Slot, SlotTypeEnum> StackContext::topWithSlotType() const {
        return std::make_tuple(memory[sp], memoryType[sp]);
    }

    void StackContext::pop(i4 size) {
        sp -= size;
    }

    void StackContext::reset() {
        sp = -1;
    }

     Slot StackContext::getStackOffset(size_t offset) const {
         return memory[sp - offset];
     }

    std::vector<Oop *> StackContext::getObjects() const {
        std::vector<Oop *> result;
        for (auto i = 0; i <= sp; ++i) {
            const auto &val = memory[i];
            const auto valType = memoryType[i];
            if (valType == SlotTypeEnum::REF && val.refVal != nullptr) {
                result.emplace_back(val.refVal);
            }
        }

        return result;
    }

    [[nodiscard]] Slot *StackContext::getCurrentSlotPtr() const {
        return memory + sp;
    }

    [[nodiscard]] SlotTypeEnum *StackContext::getCurrentSlotTypePtr() const {
        return memoryType + sp;
    }

    void StackContext::dup() {
        push(topWithSlotType());
    }

    void StackContext::dup_x1() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val1);
        push(val2);
        push(val1);
    }

    void StackContext::dup_x2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        push(val1);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::dup2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val2);
        push(val1);
        push(val2);
        push(val1);
    }

    void StackContext::dup2_x1() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        push(val2);
        push(val1);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::dup2_x2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        const auto val4 = popWithSlotType();
        push(val2);
        push(val1);
        push(val4);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::swapTop() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val1);
        push(val2);
    }

}